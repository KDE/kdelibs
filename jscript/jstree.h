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
#ifndef JSTREE_H
#define JSTREE_H

class JSValue;
class JSNode;
class JSScope;
class JSScopeStack;
class JSFunction;
class JSParameter;
class JSFunctionCall;
class JSConstructorCall;
class JSArgument;
class JSParameterListObject;
class JSThis;
class JSStringObject;
class JSArrayaAccess;

#include <qstring.h>

#define ID_JSNode 1
#define ID_JSInteger 2
#define ID_JSBinaryOperator 3
#define ID_JSAssignment 4
#define ID_JSIdentifier 5
#define ID_JSStatement 6
#define ID_JSFunction 7
#define ID_JSParameter 8
#define ID_JSFunctionCall 9
#define ID_JSArgument 10
#define ID_JSConstructorCall 11
#define ID_JSThis 12
#define ID_JSMember 13
#define ID_JSString 14
#define ID_JSBool 15
#define ID_JSFloat 16
#define ID_JSNull 17
#define ID_JSArrayAccess 18

class JSNode
{
public:
    JSNode();
    virtual ~JSNode() { }
    
    virtual int isA() { return ID_JSNode; }

    virtual int leftValue( JSScopeStack* _s, JSValue *lv );
    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    
};

#include "jsexec.h"

class JSStatement : public JSNode
{
public:
    JSStatement( JSNode *_code, JSNode *_next_code );
    virtual ~JSStatement() { if ( code ) delete code; if ( nextCode ) delete nextCode; }
    
    virtual int isA() { return ID_JSStatement; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );

protected:
    JSNode *code;
    JSNode *nextCode;
};

class JSInteger : public JSNode
{
public:
    JSInteger( int );
    virtual ~JSInteger() { }

    virtual int isA() { return ID_JSInteger; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    

protected:
    int value;
};

class JSBool : public JSNode
{
public:
    JSBool( bool );
    virtual ~JSBool() { }

    virtual int isA() { return ID_JSBool; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    

protected:
    bool value;
};

class JSFloat : public JSNode
{
public:
    JSFloat( double );
    virtual ~JSFloat() { }

    virtual int isA() { return ID_JSFloat; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    

protected:
    double value;
};

class JSBinaryOperator : public JSNode
{
public:
    JSBinaryOperator( int _op, JSNode *_left, JSNode *_right );
    virtual ~JSBinaryOperator() { if ( leftNode ) delete leftNode; if ( rightNode ) delete rightNode; }

    virtual int isA() { return ID_JSBinaryOperator; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );
    
protected:
    int op;
    JSNode *leftNode;
    JSNode *rightNode;
};

class JSAssignment : public JSBinaryOperator
{
public:
    JSAssignment( int _op, JSNode *_left, JSNode *_right );
    virtual ~JSAssignment() { }

    virtual int isA() { return ID_JSAssignment; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );
};

class JSIdentifier : public JSNode
{
public:
    JSIdentifier( const char *_name );
    virtual ~JSIdentifier() { }

    virtual int isA() { return ID_JSIdentifier; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    
    virtual int leftValue( JSScopeStack* _s, JSValue *rv );    

protected:
    QString name;
};

class JSFunction : public JSNode
{
public:
    JSFunction( const char *_name, JSParameter* _param, JSNode *_code );
    virtual ~JSFunction() { }

    virtual int isA() { return ID_JSFunction; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    
    virtual int rightValue( JSScopeStack* _s, JSValue *rv, JSParameterListObject *_param );    

    const char* getName() { return name.data(); }
    
protected:
    QString name;
    JSNode *code;
    JSParameter *parameters;
};

class JSParameter : public JSNode
{
public:
    /**
     * JSParameter may be 0L to indicate that there are no parameters to this
     * function.
     */
    JSParameter( const char *_name, JSParameter *_next );
    virtual ~JSParameter() { }

    virtual int isA() { return ID_JSParameter; }

    JSParameter* getNextParameter() { return nextParameter; }
    
protected:
    QString name;
    JSParameter *nextParameter;
};

class JSFunctionCall : public JSNode
{
public:
    JSFunctionCall( JSNode *_function, JSArgument *_arguments );
    virtual ~JSFunctionCall() { }
    
    virtual int isA() { return ID_JSFunctionCall; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    

protected:
    JSNode *function;
    JSArgument *arguments;
};

class JSArgument : public JSNode
{
public:
    JSArgument( JSNode *_code, JSArgument *_next );
    virtual ~JSArgument() { }

    virtual int isA() { return ID_JSArgument; }

    virtual int rightValue( JSScopeStack *_scopes, JSParameterListObject *_param );

    JSArgument* getNextArgument() { return nextArgument; }
    
protected:
    JSNode *code;
    JSArgument *nextArgument;
};

class JSConstructorCall : public JSNode
{
public:
    JSConstructorCall( JSNode *_function, JSArgument *_arguments );
    virtual ~JSConstructorCall() { }
    
    virtual int isA() { return ID_JSConstructorCall; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    

protected:
    JSNode *function;
    JSArgument *arguments;
};

class JSThis : public JSNode
{
public:
    JSThis();
    virtual ~JSThis() { }
    
    virtual int isA() { return ID_JSThis; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );
};

class JSMember : public JSNode
{
public:
    JSMember( JSNode *_obj, const char *_name );
    virtual ~JSMember();
    
    virtual int isA() { return ID_JSMember; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );
    virtual int leftValue( JSScopeStack* _s, JSValue *lv );

protected:
    QString name;
    JSNode *object;
};

class JSString : public JSNode
{
public:
    JSString( const char *_string );
    virtual ~JSString();

    virtual int isA() { return ID_JSString; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );

protected:    
    JSStringObject *object;
};

class JSNull : public JSNode
{
public:
    JSNull();
    virtual ~JSNull();

    virtual int isA() { return ID_JSNull; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );

protected:    
    JSObject *object;
};

class JSArrayAccess : public JSNode
{
public:
    JSArrayAccess( JSNode *_array, JSNode *_index );
    virtual ~JSArrayAccess() { }

    virtual int isA() { return ID_JSArrayAccess; }

    virtual int rightValue( JSScopeStack* _s, JSValue *rv );    
    virtual int leftValue( JSScopeStack* _s, JSValue *rv );    

protected:
    JSNode *index;
    JSNode *array;    
};

#endif


