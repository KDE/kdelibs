#define ____CPP____
#include "bison2cpp.h"
#include "jstree.h"
#include "jsexec.h"

#include <qlist.h>

JSCode *code;

extern "C" void* newJSInteger( int _value )
{
    return (void*) new JSInteger( _value );
}

extern "C" void* newJSBinaryOperator( int _op, void* _left, void *_right )
{
    return (void*) new JSBinaryOperator( _op, (JSNode*)_left, (JSNode*)_right );
}

extern "C" void* newJSAssignment( int _op, void* _left, void *_right )
{
    return (void*) new JSAssignment( _op, (JSNode*)_left, (JSNode*)_right );
}

extern "C" void* newJSIdentifier( char *_name )
{
    return (void*) new JSIdentifier( (const char *)_name );
}

extern "C" void* newJSStatement( void *_code, void *_next_code )
{
    return (void*) new JSStatement( (JSNode*)_code, (JSNode*)_next_code );
}

extern "C" void jsAppendCode( void *_code )
{
    code->append( (JSNode*)_code );
}

extern "C" void* newJSFunction( const char *_name, void *_param, void *_code )
{
    return (void*) new JSFunction( _name, (JSParameter*)_param, (JSNode*)_code );
}

extern "C" void* newJSParameter( const char *_name, void *_next )
{
    return (void*) new JSParameter( _name, (JSParameter*)_next );
}

extern "C" void* newJSArgument( void *_code, void *_next )
{
    return (void*) new JSArgument( (JSNode*)_code, (JSArgument*)_next );
}

extern "C" void* newJSFunctionCall( void *_function, void *_arguments )
{
    return (void*) new JSFunctionCall( (JSNode*)_function, (JSArgument*)_arguments );
}

extern "C" void* newJSConstructorCall( void *_function, void *_arguments )
{
    return (void*) new JSConstructorCall( (JSNode*)_function, (JSArgument*)_arguments );
}

extern "C" void* newJSThis()
{
    return (void*) new JSThis();
}

extern "C" void* newJSNull()
{
    return (void*) new JSNull();
}

extern "C" void* newJSMember( void *_obj, char* _member )
{
    return (void*) new JSMember( (JSNode*)_obj, (const char*)_member );
}

extern "C" void* newJSArrayAccess( void *_array, void *_index )
{
    return (void*) new JSArrayAccess( (JSNode*)_array, (JSNode*)_index );
}

extern "C" void* newJSString( char *_string )
{
    return (void*) new JSString( (const char*)_string );
}

extern "C" void* newJSBool( char _bool )
{
    return (void*) new JSBool( (bool)_bool );
}

extern "C" void* newJSFloat( double _f )
{
    return (void*) new JSFloat( _f );
}

int parseJavaScript( const char *_script, JSCode* _code, JSScope* _global )
{
    code = _code;
    mainParse( _script );

    JSNode *c;
    for ( c = _code->first(); c != 0L; c = _code->next() )
    {
	if ( c->isA() == ID_JSFunction )
	{
	    JSFunction *func = (JSFunction*)c;
	    _global->insertObject( new JSFunctionObject( func ) );
	}
    }

    return 0L;
}
