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
#ifndef JSEXEC_H
#define JSEXEC_H

class JSObject;
class JSScope;
class JSVariableObject;
class JSFunctionObject;
class JSUserDefinedObject;
class JSInstanceScope;

#include <qlist.h>
#include <qstack.h>

#include "jstree.h"
#include "jserror.h"

#define TYPE_JSObject 1
#define TYPE_JSIntegerObject 2
#define TYPE_JSStringObject 3
#define TYPE_JSVariableObject 4
#define TYPE_JSParameterListObject 5
#define TYPE_JSFunctionObject 6
#define TYPE_JSUserDefinedObject 7
#define TYPE_JSScope 8
#define TYPE_JSInstanceScope 9
#define TYPE_JSBoolObject 10
#define TYPE_JSFloatObject 11
#define TYPE_JSAbstractArrayObject 12

typedef QList<JSNode> JSCode;

int execJavaScript( JSCode *_code, JSScope *_global_scope, JSInstanceScope *_instanceScope );
int parseJavaScript( const char *_script, JSCode* _code, JSScope* _global );
JSScope* makeGlobalScope();

class JSValue
{
public:
    JSValue();
    virtual ~JSValue();
    
    bool isLeftValue() { return bLeftValue; }
    void setLeftValue( bool _b ) { bLeftValue = _b; }
    
    JSObject *getObject() { return object; }
    void setObject( JSObject *_o );
    
    void setAutoDelete( bool _b ) { bAutoDelete = _b; }
    bool isAutoDelete() { return bAutoDelete; }
    
protected:
    JSObject *object;
    bool bAutoDelete;
    bool bLeftValue;
};

class JSObject
{
public:
    JSObject() { }
    virtual ~JSObject() { }

    virtual int isA() { return TYPE_JSObject; }    
    virtual bool inherits( int _type ) { return ( _type == TYPE_JSObject ); }

    virtual JSObject* copy() { return new JSObject(); }    
    virtual void release() { delete this; }
};

/**********************************************************
 *
 * JSScope
 *
 *********************************************************/

class JSScope
{
public:
    JSScope();
    virtual ~JSScope() { }
  
    virtual int isA() { return TYPE_JSScope; }    
	
    void insertObject( JSObject* _obj );

    JSVariableObject* findVariable( const char *_name );
    JSFunctionObject* findFunction( const char *_name );
    
    QList<JSObject>* getObjectList() { return &objectList; }
    
protected:
    QList<JSObject> objectList;
};

/**********************************************************
 *
 * JSInstanceScope
 *
 *********************************************************/

class JSInstanceScope : public JSScope
{
public:
    JSInstanceScope() { }
    virtual ~JSInstanceScope() { }

    virtual int isA() { return TYPE_JSInstanceScope; }    

    void setObject( JSUserDefinedObject* _obj ) { object = _obj; }
    
    JSUserDefinedObject* getObject() { return object; }
    
protected:
    JSUserDefinedObject *object;
};

/**********************************************************
 *
 * JSScopeStack
 *
 *********************************************************/

class JSScopeStack
{
public:
    JSScopeStack( JSScope *_globalScope, JSInstanceScope *_instanceScope = 0L );
    virtual ~JSScopeStack();
  
    /**
     * @param _top_only causes only the upper most scope to be searched.
     * 
     * @see JSIdentifier::leftValue
     */
    JSVariableObject* findVariable( const char *_name, bool _top_only = FALSE );
    JSFunctionObject* findFunction( const char *_name );

    void pushScope( JSScope* _scope );
    void popScope();
    JSScope* topScope();
    
    void pushInstanceScope( JSInstanceScope* _scope );
    void popInstanceScope();
    JSInstanceScope* topInstanceScope();
    
protected:
    QList<JSScope> scopeList;
    QStack<JSInstanceScope> instanceScopeStack;

    /**
     * If this pointer is NOT 0L, then this scope may not be deleted. Mention that all other
     * scopes are deleted if @ref #scopeList is deleted.
     */
    JSScope* globalScope;    
};

class JSUserDefinedObject : public JSObject
{
public:
    JSUserDefinedObject();
    virtual ~JSUserDefinedObject() { }

    virtual int isA() { return TYPE_JSUserDefinedObject; }    
    virtual bool inherits( int _type )
    { if ( _type == TYPE_JSUserDefinedObject ) return TRUE; else return JSObject::inherits( _type ); }

    virtual JSObject* copy() { lock++; return this; }
    virtual void release() { if ( lock == 1 ) delete this; else lock--;  }
    
    JSInstanceScope* getScope() { return &scope; }
    
protected:
    JSInstanceScope scope;
    /**
     * Amount of references hold to this object.
     *
     * @see copy
     * @see release
     */
    int lock;
};

/**
 * @short Abstract class for Arrays.
 *
 * This class is only an abstract class. If your object is going to accept the "ident[ index ]"
 * Operator then you have to derive from this class.
 *
 * @see JSAbstractArrayAccess
 */
class JSAbstractArrayObject : public JSUserDefinedObject
{
public:
    JSAbstractArrayObject() { }
    virtual ~JSAbstractArrayObject() { }

    virtual int isA() { return TYPE_JSAbstractArrayObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSAbstractArrayObject ) return TRUE;
                                         else return JSUserDefinedObject::inherits( _type ); }
    
    virtual int rightValue( JSObject *, JSValue * ) { return ERROR_JSNotARightValue; }
    virtual int leftValue( JSObject *, JSValue * ) { return ERROR_JSNotALeftValue; }
};

class JSStringObject : public JSUserDefinedObject
{
public:
    JSStringObject( const char *_string );
    virtual ~JSStringObject() { }

    virtual int isA() { return TYPE_JSStringObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSStringObject ) return TRUE;
                                         else return JSUserDefinedObject::inherits( _type ); }

    const char* getString() { return string.data(); }
    QString& getQString() { return string; }

protected:
    QString string;
};

class JSParameterListObject : public JSObject
{
public:
    JSParameterListObject();
    virtual ~JSParameterListObject() { }

    virtual int isA() { return TYPE_JSParameterListObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSParameterListObject ) return TRUE;
                                         else return JSObject::inherits( _type ); }

    void appendValue( JSValue* _val ) { parameterValues.append( _val ); }

    JSValue* firstValue() { return parameterValues.first(); }
    JSValue* nextValue() { return parameterValues.next(); }
    
protected:
    QList<JSValue> parameterValues;
};

class JSFunctionObject : public JSObject
{
public:
    JSFunctionObject( JSFunction *_func );
    virtual ~JSFunctionObject() { }

    virtual int isA() { return TYPE_JSFunctionObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSFunctionObject ) return TRUE;
                                         else return JSObject::inherits( _type ); }

    virtual JSObject* copy() { return new JSFunctionObject( function ); }    

    JSFunction* getFunction() { return function; }
    const char* getName();
    
    void setObject( JSUserDefinedObject *_obj ) { object = _obj; }
    JSUserDefinedObject* getObject() { return object; }
    
protected:
    JSFunction *function;
    JSUserDefinedObject *object;
};

class JSVariableObject : public JSObject
{
public:
    JSVariableObject();
    virtual ~JSVariableObject();

    virtual int isA() { return TYPE_JSVariableObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSVariableObject ) return TRUE;
                                         else return JSObject::inherits( _type ); }

    virtual void setName( const char *_name ) {	if ( bConst ) return; name = _name; }
    virtual const char* getName() { return name.data(); }

    virtual JSObject *getValue() { return value; }
    virtual void setValue( JSObject* _val ) { if ( bConst ) return; value = _val; }

    virtual void clear();
    
    /**
     * If a variable is const, then you can not assign anything to this variable. Use this to
     * implement for example builtin objects like "document".
     * This function is for extensions only, it is not called by the interpreter.
     */
    void setConst( bool _c ) { bConst = _c; }
    bool isConst() { return bConst; }

    /**
     * Set this flag if the value of this variable changes sometimes ( examples: time, status bar, ... ).
     * If this flag is set, the object returned from @ref #getValue will be deleted if not
     * further used.
     */
    void setDynamic( bool _c ) { bDynamic = _c; }
    bool isDynamic() { return bDynamic; }
    
protected:
    QString name;

    JSObject *value;

    bool bConst;
    bool bDynamic;
};

/* class JSStringObject : public JSObject
{
public:
    JSStringObject( const char *_str );
    virtual ~JSStringObject() { }

    virtual int isA() { return TYPE_JSStringObject; }    

protected:
    QString string;
}; */

class JSIntegerObject : public JSObject
{
public:
    JSIntegerObject( int _i );
    JSIntegerObject() { }

    virtual int isA() { return TYPE_JSIntegerObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSIntegerObject ) return TRUE;
                                         else return JSObject::inherits( _type ); }

    virtual JSObject* copy() { return new JSIntegerObject( value ); }    

    int getValue() { return value; }
    
protected:
    int value;
};

class JSBoolObject : public JSObject
{
public:
    JSBoolObject( bool _i );
    JSBoolObject() { }

    virtual int isA() { return TYPE_JSBoolObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSBoolObject ) return TRUE;
                                         else return JSObject::inherits( _type ); }

    virtual JSObject* copy() { return new JSBoolObject( value ); }    

    bool getValue() { return value; }
    
protected:
    bool value;
};

class JSFloatObject : public JSObject
{
public:
    JSFloatObject( double _i );
    JSFloatObject() { }

    virtual int isA() { return TYPE_JSFloatObject; }    
    virtual bool inherits( int _type ) { if ( _type == TYPE_JSFloatObject ) return TRUE;
                                         else return JSObject::inherits( _type ); }

    virtual JSObject* copy() { return new JSFloatObject( value ); }    

    double getValue() { return value; }
    
protected:
    double value;
};

#endif
