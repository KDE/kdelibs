/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Javascript
// $Id$

#ifndef JSBUILTIN_H
#define JSBUILTIN_H

#include <qstring.h>
#include <jsexec.h>

//
// External classes
//
///////////////////
class KHTMLView;
class KHTMLWidget;

//
// Internal classes
//
///////////////////
class JSEnvironment;
class JSDocumentObject;
class JSDocumentWriteFunction;
class JSWindowObject;
class JSWindowVarObject;
class JSInstanceScope;
class JSWindowArrayObject;

class JSEventHandler
{
public:
    JSEventHandler( JSEnvironment *_env, const char *_name, const char *_code );
    ~JSEventHandler();
    
    JSEnvironment *getJSEnvironment() { return jsEnvironment; }
    const char* getName() { return name.data(); }
    int exec( JSInstanceScope *_scope );
    
protected:
    JSCode *code;
    QString name;
    JSEnvironment *jsEnvironment;
};

#define TYPE_JSDocumentObject 1000
#define TYPE_JSWindowObject 1001
#define TYPE_JSWindowVarObject 1002

#define KIND_JSWindowStatus 1
#define KIND_JSWindowParent 2
#define KIND_JSWindowTop 3
#define KIND_JSWindowName 4
#define KIND_JSWindowFrames 5

class JSWindowObject : public JSUserDefinedObject
{
public:
    JSWindowObject( JSEnvironment *_jsenv );
    virtual ~JSWindowObject();

    virtual int isA() { return TYPE_JSWindowObject; }    
    virtual bool inherits( int _type )
    { if ( _type == TYPE_JSWindowObject ) return TRUE;
    else return JSUserDefinedObject::inherits( _type ); }

    /**
     * Never delete this instance.
     */
    virtual void release() { if ( lock > 1 ) lock--;  }

    JSEnvironment* getJSEnvironment() { return jsEnvironment; }
    
protected:
    /**
     * The variable 'status' of the window object.
     */
    JSWindowVarObject *varStatus;
    JSWindowVarObject *varParent;
    JSWindowVarObject *varTop;    
    JSWindowVarObject *varName;

    /**
     * Pointer to the environment this window object belongs to.
     */
    JSEnvironment *jsEnvironment;

    /**
     * The object 'document'
     */
    JSDocumentObject *jsDocumentObject;
    /**
     * The variable that holds the object 'document'.
     */
    JSVariableObject *jsDocumentVar;
    /**
     * The object 'frames'
     */
    JSWindowArrayObject *jsFramesObject;
    /**
     * The variable that holds the array 'frames'.
     */
    JSVariableObject *jsFramesVar;

};

class JSWindowArrayObject : public JSAbstractArrayObject
{
public:
    JSWindowArrayObject( JSWindowObject *_win, int _kind );
    virtual ~JSWindowArrayObject();

    virtual int rightValue( JSObject *_index, JSValue *_rv );

protected:
    int kind;
    JSWindowObject *object;
};

class JSWindowVarObject : public JSVariableObject
{
public:
    JSWindowVarObject( JSWindowObject *_win, int _kind );
    virtual ~JSWindowVarObject();

    virtual JSObject *getValue();
    virtual void setValue( JSObject* _val );
    virtual void clear();

protected:
    int kind;
    JSWindowObject *object;
};

class JSDocumentObject : public JSUserDefinedObject
{
public:
    JSDocumentObject( JSWindowObject *_window );
    virtual ~JSDocumentObject();

    virtual int isA() { return TYPE_JSDocumentObject; }    
    virtual bool inherits( int _type )
    { if ( _type == TYPE_JSDocumentObject ) return TRUE;
    else return JSUserDefinedObject::inherits( _type ); }

    /**
     * Never delete this instance.
     */
    virtual void release() { if ( lock > 1 ) lock--;  }

    JSWindowObject* getJSWindowObject() { return window; }
    
protected:
    JSDocumentWriteFunction *jsDocumentWrite;
    JSWindowObject *window;
};

class JSDocumentWriteFunction : public JSFunction
{
public:
    JSDocumentWriteFunction( JSDocumentObject *_obj );
    virtual ~JSDocumentWriteFunction();
    
    virtual int rightValue( JSScopeStack* _s, JSValue *rv, JSParameterListObject *_param );

protected:
    JSDocumentObject *object;
};

class JSEnvironment
{
public:
    JSEnvironment( KHTMLWidget *_view );
    ~JSEnvironment();
    
    JSCode* parse( const char *_script );
    int exec( JSCode *_code, JSInstanceScope *_scope = 0L );
    
    const char* readOutput() { return output.data(); }
    void writeOutput( const char *_str ) { output += _str; }

    JSWindowObject* getJSWindowObject() { return jsWindowObject; }
    KHTMLWidget* getKHTMLWidget() { return view; }
    KHTMLView* getKHTMLView();
    
protected:
    KHTMLWidget *view;
    JSScope *globalScope;
    
    JSWindowObject *jsWindowObject;

    QString output;
};

#endif
