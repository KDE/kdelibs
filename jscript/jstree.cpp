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
#include "jstree.h"
#include "jsexec.h"
#include "jserror.h"
#include "bison2cpp.h"
#include <stdio.h>

/**********************************************************
 *
 * JSNode
 *
 *********************************************************/

JSNode::JSNode()
{
}

int JSNode::leftValue( JSScopeStack *, JSValue * )
{
    return ERROR_JSNotALeftValue;
}

int JSNode::rightValue( JSScopeStack *, JSValue * )
{
    return ERROR_JSNotARightValue;
}

/**********************************************************
 *
 * JSInteger
 *
 *********************************************************/

JSInteger::JSInteger( int _value ) : JSNode()
{
    value = _value;
}

int JSInteger::rightValue( JSScopeStack * , JSValue *_rv )
{
    _rv->setObject( new JSIntegerObject( value ) );
    _rv->setAutoDelete( TRUE );
    _rv->setLeftValue( FALSE );

    return 0;
}

/**********************************************************
 *
 * JSBool
 *
 *********************************************************/

JSBool::JSBool( bool _value ) : JSNode()
{
    value = _value;
}

int JSBool::rightValue( JSScopeStack *, JSValue *_rv )
{
    _rv->setObject( new JSBoolObject( value ) );
    _rv->setAutoDelete( TRUE );
    _rv->setLeftValue( FALSE );

    return 0;
}

/**********************************************************
 *
 * JSFloat
 *
 *********************************************************/

JSFloat::JSFloat( double _value ) : JSNode()
{
    value = _value;
}

int JSFloat::rightValue( JSScopeStack *, JSValue *_rv )
{
    _rv->setObject( new JSFloatObject( value ) );
    _rv->setAutoDelete( TRUE );
    _rv->setLeftValue( FALSE );

    return 0;
}

/**********************************************************
 *
 * JSBinaryOperator
 *
 *********************************************************/

JSBinaryOperator::JSBinaryOperator( int _op, JSNode *_left, JSNode *_right ) : JSNode()
{
    op = _op;
    leftNode = _left;
    rightNode = _right;
}

int JSBinaryOperator::rightValue( JSScopeStack *_scope, JSValue *_val )
{
    int ret = 0;

    if ( !leftNode || !rightNode )
	return ERROR_JSInternal;
    
    JSValue *lv = new JSValue();
    ret = leftNode->rightValue( _scope, lv );
    if ( ret )
    {
	delete lv;
	return ret;
    }

    JSValue* rv = new JSValue();
    ret = rightNode->rightValue( _scope, rv );
    if ( ret )
    {
	delete lv;
	delete rv;
	return ret;
    }

    // Is at least one of the operands null ?
    if ( lv->getObject()->isA() == TYPE_JSObject || rv->getObject()->isA() == TYPE_JSObject )
    {
	_val->setLeftValue( FALSE );    
	_val->setAutoDelete( TRUE );
	JSObject *obj;
	switch ( op )
	{
	case OP_EQ:
	    obj = new JSBoolObject( lv->getObject()->isA() == rv->getObject()->isA() );
	    break;
	case OP_NEQ:
	    obj = new JSBoolObject( lv->getObject()->isA() != rv->getObject()->isA() );
	    break;
	default:
	    delete lv;
	    delete rv;
	    return ERROR_JSOperatorNotAllowed;
	}
	_val->setObject( obj );
	delete lv;
	delete rv;
	return ret;
    }

    switch ( lv->getObject()->isA() )
    {
    case TYPE_JSIntegerObject:
	{
	    JSIntegerObject *i1 = (JSIntegerObject*) lv->getObject();
	    // TODO: Convert rv to integer;
	    JSIntegerObject *i2 = (JSIntegerObject*) rv->getObject();
	    _val->setLeftValue( FALSE );    
	    _val->setAutoDelete( TRUE );
	    JSObject *obj;
	    switch ( op )
	    {
	    case OP_ADD:
		obj = new JSIntegerObject( i1->getValue() + i2->getValue() );
	        break;
	    case OP_MUL:
		obj = new JSIntegerObject( i1->getValue() * i2->getValue() );
	        break;
	    case OP_DIV:
		obj = new JSIntegerObject( i1->getValue() / i2->getValue() );
	        break;
	    case OP_SUB:
		obj = new JSIntegerObject( i1->getValue() - i2->getValue() );
	        break;
	    case OP_EQ:
		obj = new JSBoolObject( i1->getValue() == i2->getValue() );
	        break;
	    case OP_NEQ:
		obj = new JSBoolObject( i1->getValue() != i2->getValue() );
	        break;
	    case OP_LEQ:
		obj = new JSBoolObject( i1->getValue() <= i2->getValue() );
	        break;
	    case OP_GEQ:
		obj = new JSBoolObject( i1->getValue() >= i2->getValue() );
	        break;
	    case OP_LT:
		obj = new JSBoolObject( i1->getValue() < i2->getValue() );
	        break;
	    case OP_GT:
		obj = new JSBoolObject( i1->getValue() > i2->getValue() );
	        break;
	    case OP_SL:
		obj = new JSIntegerObject( i1->getValue() << i2->getValue() );
	        break;
	    case OP_SR:
		obj = new JSIntegerObject( i1->getValue() >> i2->getValue() );
	        break;
	    case OP_BAND:
		obj = new JSIntegerObject( i1->getValue() & i2->getValue() );
	        break;
	    case OP_BOR:
		obj = new JSIntegerObject( i1->getValue() | i2->getValue() );
	        break;
	    case OP_BXOR:
		obj = new JSIntegerObject( i1->getValue() ^ i2->getValue() );
	        break;
	    default:
		delete lv;
		delete rv;
		return ERROR_JSOperatorNotAllowed;
	    }
	    _val->setObject( obj );
	}
        break;
    case TYPE_JSFloatObject:
	{
	    JSFloatObject *i1 = (JSFloatObject*) lv->getObject();
	    // TODO: Convert rv to float;
	    JSFloatObject *i2 = (JSFloatObject*) rv->getObject();
	    _val->setLeftValue( FALSE );    
	    _val->setAutoDelete( TRUE );
	    JSObject *obj;
	    switch ( op )
	    {
	    case OP_ADD:
		obj = new JSFloatObject( i1->getValue() + i2->getValue() );
	        break;
	    case OP_MUL:
		obj = new JSFloatObject( i1->getValue() * i2->getValue() );
	        break;
	    case OP_DIV:
		obj = new JSFloatObject( i1->getValue() / i2->getValue() );
	        break;
	    case OP_SUB:
		obj = new JSFloatObject( i1->getValue() - i2->getValue() );
	        break;
	    case OP_EQ:
		obj = new JSBoolObject( i1->getValue() == i2->getValue() );
	        break;
	    case OP_NEQ:
		obj = new JSBoolObject( i1->getValue() != i2->getValue() );
	        break;
	    case OP_LEQ:
		obj = new JSBoolObject( i1->getValue() <= i2->getValue() );
	        break;
	    case OP_GEQ:
		obj = new JSBoolObject( i1->getValue() >= i2->getValue() );
	        break;
	    case OP_LT:
		obj = new JSBoolObject( i1->getValue() < i2->getValue() );
	        break;
	    case OP_GT:
		obj = new JSBoolObject( i1->getValue() > i2->getValue() );
	        break;
	    default:
		delete lv;
		delete rv;
		return ERROR_JSOperatorNotAllowed;
	    }
	    _val->setObject( obj );
	}
        break;
    case TYPE_JSStringObject:
	{
	    JSStringObject *s1 = (JSStringObject*) lv->getObject();
	    // TODO: Convert rv to integer;
	    JSStringObject *s2 = (JSStringObject*) rv->getObject();
	    _val->setLeftValue( FALSE );    
	    switch ( op )
	    {
	    case OP_ADD:
		{
		    _val->setAutoDelete( TRUE );
		    JSStringObject *obj = new JSStringObject( s1->getQString() + s2->getQString() );
		    _val->setObject( obj );
		}
	        break;
	    case OP_EQ:
		{
		    _val->setAutoDelete( TRUE );
		    JSBoolObject *obj = new JSBoolObject( strcmp( s1->getString(), s2->getString() ) == 0L );
		    _val->setObject( obj );
		}
	        break;
	    case OP_NEQ:
		{
		    _val->setAutoDelete( TRUE );
		    JSBoolObject *obj = new JSBoolObject( strcmp( s1->getString(), s2->getString() ) != 0L );
		    _val->setObject( obj );
		}
	        break;
	    default:
		delete lv;
		delete rv;
		return ERROR_JSOperatorNotAllowed;
	    }
	}
        break;
    case TYPE_JSBoolObject:
	{
	    JSBoolObject *s1 = (JSBoolObject*) lv->getObject();
	    // TODO: Convert rv to integer;
	    JSBoolObject *s2 = (JSBoolObject*) rv->getObject();
	    _val->setLeftValue( FALSE );    
	    _val->setAutoDelete( TRUE );
	    JSObject *obj;
	    switch ( op )
	    {
	    case OP_EQ:
		obj = new JSBoolObject( s1->getValue() == s2->getValue() );
	        break;
	    case OP_NEQ:
		obj = new JSBoolObject( s1->getValue() != s2->getValue() );
	        break;
	    case OP_AND:
		obj = new JSBoolObject( s1->getValue() && s2->getValue() );
	        break;
	    case OP_OR:
		obj = new JSBoolObject( s1->getValue() || s2->getValue() );
	        break;
	    default:
		delete lv;
		delete rv;
		return ERROR_JSOperatorNotAllowed;
	    }
	    _val->setObject( obj );
	}
        break;
    default:
	delete lv;
	delete rv;
	return ERROR_JSOperatorNotAllowed;
    }
    
    delete lv;
    delete rv;
    return ret;
}

/**********************************************************
 *
 * JSAssignment
 *
 *********************************************************/

JSAssignment::JSAssignment( int _op, JSNode *_left, JSNode *_right ) : 
    JSBinaryOperator( _op, _left, _right )
{
}

int JSAssignment::rightValue( JSScopeStack *_scopes, JSValue * )
{
    int ret = 0;
    if ( !leftNode || !rightNode )
	return ERROR_JSInternal;
    
    JSValue *lv = new JSValue();
    ret = leftNode->leftValue( _scopes, lv );
    if ( ret )
    {
	delete lv;
	return ret;
    }

    JSValue* rv = new JSValue();
    ret = rightNode->rightValue( _scopes, rv );
    if ( ret )
    {
	delete lv;
	delete rv;
	return ret;
    }
    
    if ( !lv->isLeftValue() )
    {
	delete lv;
	delete rv;
	return ERROR_JSNotALeftValue;
    }

    // We can only assign to JSVariableOvbjects.
    if ( lv->getObject()->inherits( TYPE_JSVariableObject ) )
    {
	JSVariableObject *var = (JSVariableObject*)(lv->getObject() );
	if ( var->isConst() )
	{
	    delete lv;
	    delete rv;
	    return ERROR_JSNotALeftValue;
	}
	var->clear();
	if ( rv->isAutoDelete() )
	{
	    // 'rv' should not delete the object since we reuse it in 'var'
	    rv->setAutoDelete( FALSE );
	    var->setValue( rv->getObject() );
	}
	else
	    var->setValue( rv->getObject()->copy() );
    }
    else
    {
	delete lv;
	delete rv;
	return ERROR_JSInternal;
    }
    
    delete lv;
    delete rv;
    
    return ret;
}

/**********************************************************
 *
 * JSIdentifier
 *
 *********************************************************/

JSIdentifier::JSIdentifier( const char *_name )
{
    name = _name;
    name.detach();
}

int JSIdentifier::rightValue( JSScopeStack *_scopes, JSValue *_rv )
{
    // int ret = 0;
 
    // Is it a function ?
    JSFunctionObject* func;
    func = _scopes->findFunction( name );
    if ( func )
    {
	_rv->setObject( func );
	_rv->setAutoDelete( FALSE );
	_rv->setLeftValue( FALSE );
	return 0;
    }
    
    // Is it a variable ?
    JSVariableObject* var;
    var = _scopes->findVariable( name, FALSE );
    if ( !var )
	return ERROR_JSUnknownIdentifier;
    
    _rv->setObject( var->getValue() );
    if ( var->isDynamic() )
	_rv->setAutoDelete( TRUE );
    else
	_rv->setAutoDelete( FALSE );
    _rv->setLeftValue( FALSE );

    return 0;
}

int JSIdentifier::leftValue( JSScopeStack *_scopes, JSValue *_lv )
{
    JSVariableObject* var;
    
    // int ret = 0;
    
    var = _scopes->findVariable( name, TRUE );
    if ( !var )
    {
	JSScope* scope = _scopes->topScope();
	scope->insertObject( var = new JSVariableObject() );
	var->setName( name );
    }
    
    _lv->setObject( var );
    _lv->setAutoDelete( FALSE );
    _lv->setLeftValue( TRUE );

    return 0;
}

/**********************************************************
 *
 * JSStatement
 *
 *********************************************************/

JSStatement::JSStatement( JSNode *_code, JSNode *_next_code ) : JSNode()
{
    code = _code;
    nextCode = _next_code;
}

int JSStatement::rightValue( JSScopeStack *_scopes, JSValue *_rv )
{
    int ret = 0;
    
    if ( code )
    {
	ret = code->rightValue( _scopes, _rv );
	if ( ret )
	    return ret;
    }
    
    if ( nextCode )
	ret = nextCode->rightValue( _scopes, _rv );
    
    return ret;
}

/**********************************************************
 *
 * JSFunction
 *
 *********************************************************/

JSFunction::JSFunction( const char *_name, JSParameter* _param, JSNode *_code ) : JSNode()
{
    parameters = _param;
    code = _code;
    name = _name;
}

int JSFunction::rightValue( JSScopeStack* , JSValue *rv )
{
    // This is NOT a call to this function. The programmer just wants a
    // reference to this function ( read: pointer ).

    JSFunctionObject *func = new JSFunctionObject( this );
    
    rv->setObject( func );
    rv->setAutoDelete( TRUE );
    rv->setLeftValue( FALSE );
    
    return 0;
}

int JSFunction::rightValue( JSScopeStack *_scopes, JSValue *_rv, JSParameterListObject* )
{
    // TODO: Scope change and fill parameters

    return code->rightValue( _scopes, _rv );
}

/**********************************************************
 *
 * JSParameter
 *
 *********************************************************/

JSParameter::JSParameter( const char *_name, JSParameter *_next )
{
    name = _name;
    nextParameter = _next;
}

/**********************************************************
 *
 * JSFunctionCall
 *
 *********************************************************/

JSFunctionCall::JSFunctionCall( JSNode *_function, JSArgument *_arguments )
{
    function = _function;
    arguments = _arguments;
}

int JSFunctionCall::rightValue( JSScopeStack* _scopes, JSValue *rv )
{
    int ret = 0;
    
    JSValue v;

    ret = function->rightValue( _scopes, &v );
    if ( ret )
	return ret;
    if ( ! v.getObject()->inherits( TYPE_JSFunctionObject ) )
    {
	printf("isA=%i\n",v.getObject()->isA());
	return ERROR_JSNotAFunction;
    }
    
    JSParameterListObject param;

    if ( arguments )
    {
	ret = arguments->rightValue( _scopes, &param );
	if ( ret )
	    return ret;
    }
    
    JSFunctionObject *func = (JSFunctionObject*)( v.getObject() );

    // Add new temporary scope
    _scopes->pushScope( new JSScope() );

    if ( func->getObject() )
    {
	// Add obj to scope
	_scopes->pushInstanceScope( func->getObject()->getScope() );
    }
    
    ret = func->getFunction()->rightValue( _scopes, rv, &param );

    if ( func->getObject() )
    {
	// Remove obj from scope
	_scopes->popInstanceScope();
    }
  
    // Remove temporary scope
    _scopes->popScope();

    return ret;
}

/**********************************************************
 *
 * JSConstructorCall
 *
 *********************************************************/

JSConstructorCall::JSConstructorCall( JSNode *_function, JSArgument *_arguments )
{
    function = _function;
    arguments = _arguments;
}

int JSConstructorCall::rightValue( JSScopeStack* _scopes, JSValue *rv )
{
    int ret = 0;
    
    JSValue v;

    ret = function->rightValue( _scopes, &v );
    if ( ret )
	return ret;
    if ( ! v.getObject()->inherits( TYPE_JSFunctionObject ) )
	return ERROR_JSNotAFunction;
    
    JSParameterListObject param;

    if ( arguments )
    {
	ret = arguments->rightValue( _scopes, &param );
	if ( ret )
	    return ret;
    }

    JSFunctionObject *func = (JSFunctionObject*)( v.getObject() );

    JSUserDefinedObject *obj = new JSUserDefinedObject();

    // Add obj to scope
    _scopes->pushInstanceScope( obj->getScope() );
    // Add new temporary scope
    _scopes->pushScope( new JSScope() );
    
    ret = func->getFunction()->rightValue( _scopes, &v, &param );
    if ( ret )
    {
	delete obj;
	return ret;
    }

    // Remove temporary scope
    _scopes->popScope();
    // Remove obj from scope
    _scopes->popInstanceScope();
    
    rv->setObject( obj );
    rv->setAutoDelete( TRUE );
    rv->setLeftValue( FALSE );

    return ret;
}

/**********************************************************
 *
 * JSArgument
 *
 *********************************************************/

JSArgument::JSArgument( JSNode *_code, JSArgument *_next ) : JSNode()
{
    code = _code;
    nextArgument = _next;
}

int JSArgument::rightValue( JSScopeStack *_scopes, JSParameterListObject *_param )
{
    int ret = 0;
    
    JSValue *v = new JSValue();
    ret = code->rightValue( _scopes, v );
    if ( ret )
	return ret;
    
    _param->appendValue( v );
    
    if ( nextArgument )
	ret = nextArgument->rightValue( _scopes, _param );
    
    return ret;
}

/**********************************************************
 *
 * JSThis
 *
 *********************************************************/

JSThis::JSThis() : JSNode()
{
}

int JSThis::rightValue( JSScopeStack* _scopes, JSValue *rv )
{
    int ret = 0;
    
    JSInstanceScope *s = _scopes->topInstanceScope();
    if ( s == 0L )
	return ERROR_JSNoInstance;
	
    rv->setObject( s->getObject() );
    rv->setAutoDelete( FALSE );
    rv->setLeftValue( FALSE );
    
    return ret;
}

/**********************************************************
 *
 * JSMember
 *
 *********************************************************/

JSMember::JSMember( JSNode *_obj, const char *_name )
{
    object = _obj;
    name = _name;
}

JSMember::~JSMember()
{
    if ( object ) 
	delete object;
}
    
int JSMember::rightValue( JSScopeStack* _s, JSValue *rv )
{
    int ret = 0;
    
    JSValue v;
    ret = object->rightValue( _s, &v );
    if ( ret )
	return ret;
    
    if ( ! v.getObject()->inherits( TYPE_JSUserDefinedObject ) )
	return ERROR_JSUnknownIdentifier;
    
    JSInstanceScope *s = ((JSUserDefinedObject*)v.getObject())->getScope();

    // Test for variables.
    // Mention that variables may be function pointers, too.
    JSVariableObject* var = s->findVariable( name );
    if ( var )
    {
	// Is it a reference to a function ?
	if ( var->getValue()->inherits( TYPE_JSFunctionObject ) )
	{
	    JSFunctionObject *f = (JSFunctionObject*)( var->getValue()->copy() );
	    f->setObject( (JSUserDefinedObject*)(v.getObject()) );
	    rv->setObject( f );
	    rv->setAutoDelete( TRUE );
	    rv->setLeftValue( FALSE );
	    return ret;
	}
	// It is a usual variable
	rv->setObject( var->getValue() );
	if ( var->isDynamic() )
	    rv->setAutoDelete( TRUE );
	else
	    rv->setAutoDelete( FALSE );
	rv->setLeftValue( FALSE );
	return ret;
    }

    // Test for hard coded functions
    JSFunctionObject* func = s->findFunction( name );
    if ( !func )
	return ERROR_JSUnknownIdentifier;

    rv->setObject( func );
    rv->setAutoDelete( FALSE );
    rv->setLeftValue( FALSE );
    
    return ret;
}

int JSMember::leftValue( JSScopeStack* _s, JSValue *rv )
{
    int ret = 0;
    
    JSValue v;
    ret = object->rightValue( _s, &v );
    if ( ret )
	return ret;
    
    if ( ! v.getObject()->inherits( TYPE_JSUserDefinedObject ) )
	return ERROR_JSUnknownIdentifier;
    
    JSInstanceScope *s = ((JSUserDefinedObject*)v.getObject())->getScope();

    // Test for variables.
    // Mention that variables may be function pointers, too.
    JSVariableObject* var = s->findVariable( name );
    if ( var )
    {
	rv->setObject( var );
	rv->setAutoDelete( FALSE );
	rv->setLeftValue( TRUE );
	return ret;
    }

    // Hard coded functions can not be overwritten
    JSFunctionObject* func = s->findFunction( name );
    if ( func )
    {
	rv->setObject( func );
	rv->setAutoDelete( FALSE );
	rv->setLeftValue( FALSE );
	return ERROR_JSNotALeftValue;
    }
    
    // Insert new variable
    s->insertObject( var = new JSVariableObject() );
    var->setName( name );
    rv->setObject( var );
    rv->setAutoDelete( FALSE );
    rv->setLeftValue( TRUE );
    
    return ret;
}

/**********************************************************
 *
 * JSString
 *
 *********************************************************/

JSString::JSString( const char *_string )
{
    object = new JSStringObject( _string );
}

JSString::~JSString()
{
    delete object;
}

int JSString::rightValue( JSScopeStack* , JSValue *rv )
{
    int ret = 0;
 
    rv->setObject( object );
    rv->setLeftValue( FALSE );
    rv->setAutoDelete( FALSE );
    
    return ret;
}

/**********************************************************
 *
 * JSNull
 *
 *********************************************************/

JSNull::JSNull()
{
    object = new JSObject();
}

JSNull::~JSNull()
{
    delete object;
}

int JSNull::rightValue( JSScopeStack*, JSValue *rv )
{
    int ret = 0;
 
    rv->setObject( object );
    rv->setLeftValue( FALSE );
    rv->setAutoDelete( FALSE );
    
    return ret;
}

/**********************************************************
 *
 * JSArrayAccess
 *
 *********************************************************/

JSArrayAccess::JSArrayAccess( JSNode *_array, JSNode *_index )
{
    array = _array;
    index = _index;
}

int JSArrayAccess::rightValue( JSScopeStack* _scopes, JSValue *rv )
{
    int ret = 0;
    
    JSValue v;

    ret = array->rightValue( _scopes, &v );
    if ( ret )
	return ret;
    if ( ! v.getObject()->inherits( TYPE_JSAbstractArrayObject ) )
    {
	printf("isA=%i\n",v.getObject()->isA());
	return ERROR_JSNotAFunction;
    }
        
    JSAbstractArrayObject *a = (JSAbstractArrayObject*)( v.getObject() );
    
    JSValue iv;
    ret = index->rightValue( _scopes, &iv );
    if ( ret )
	return ret;
    
    ret = a->rightValue( iv.getObject(), rv );

    return ret;
}

int JSArrayAccess::leftValue( JSScopeStack* _scopes, JSValue *rv )
{
    int ret = 0;
    
    JSValue v;

    ret = array->rightValue( _scopes, &v );
    if ( ret )
	return ret;
    if ( ! v.getObject()->inherits( TYPE_JSAbstractArrayObject ) )
    {
	printf("isA=%i\n",v.getObject()->isA());
	return ERROR_JSNotAFunction;
    }
        
    JSAbstractArrayObject *a = (JSAbstractArrayObject*)( v.getObject() );
    
    JSValue iv;
    ret = index->rightValue( _scopes, &iv );
    if ( ret )
	return ret;
    
    ret = a->leftValue( iv.getObject(), rv );

    return ret;
}

