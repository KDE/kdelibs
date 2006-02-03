#include "static_binding.h"
#include "value_binding.h"
#include <kjs/interpreter.h>
#include <kjs/function_object.h>
#include <qdebug.h>

namespace KJSEmbed {
    static QHash<QString,const Constructor*> g_ctorHash;
}

using namespace KJSEmbed;

StaticBinding::StaticBinding(KJS::ExecState *exec, const Method *method )
    : KJS::InternalFunctionImp(static_cast<KJS::FunctionPrototype*>(exec->lexicalInterpreter()->builtinFunctionPrototype())),
      m_method(method)
{
    putDirect( KJS::lengthPropertyName, m_method->argc, LengthFlags );
    setFunctionName( m_method->name );
}

KJS::JSValue *StaticBinding::callAsFunction( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    if( m_method->call == 0 )
    {
        //throwError(exec, "Bad method id");      // NOTE: fix
        KJS::throwError(exec, KJS::GeneralError, "Bad method id");
        return KJS::Null();
    }
    return (*m_method->call)(exec,self,args);
}

void StaticBinding::publish( KJS::ExecState *exec, KJS::JSObject *object, const Method *methods )
{
    int idx = 0;
    while( methods[idx].name != 0 )
    {
        object->put(exec, methods[idx].name, new StaticBinding(exec,  &methods[idx]), methods[idx].flags);
        idx++;
    }
}

StaticConstructor::StaticConstructor(KJS::ExecState *exec, const Constructor *constructor )
    : KJS::InternalFunctionImp(static_cast<KJS::FunctionPrototype*>(exec->lexicalInterpreter()->builtinFunctionPrototype())),
      m_constructor( constructor )
{
    putDirect( KJS::lengthPropertyName, m_constructor->argc, LengthFlags );
    setFunctionName( m_constructor->name );
    m_default = KJS::Null();
}

KJS::JSObject *StaticConstructor::construct( KJS::ExecState *exec, const KJS::List &args )
{
    return (*m_constructor->construct)(exec,args);
}

void StaticConstructor::setDefaultValue( KJS::JSValue *value )
{
    m_default = value;
}

KJS::JSValue *StaticConstructor::defaultValue( KJS::ExecState * exec, KJS::Type hint ) const
{
    Q_UNUSED(exec);
    Q_UNUSED(hint);
    return m_default;
}

KJS::JSObject *StaticConstructor::add( KJS::ExecState *exec, KJS::JSObject *object, const Constructor *constructor )
{
    KJS::JSObject *obj = new StaticConstructor(exec,  constructor );
    object->put(exec, constructor->name, obj);
    if( constructor->staticMethods )
    {
        StaticBinding::publish( exec, obj, constructor->staticMethods );
    }
    /* crashes for some reason */
    if( constructor->enumerators )
    {
        int idx = 0;
        while( constructor->enumerators[idx].name != 0 )
        {
            obj->put( exec, constructor->enumerators[idx].name,
                KJS::Number(constructor->enumerators[idx].value), KJS::DontDelete|KJS::ReadOnly);
            idx++;
        }
    }
    // publish methods
    KJSEmbed::g_ctorHash[constructor->name] = constructor;
    return obj;
}

const Method *StaticConstructor::methods( const KJS::UString &className )
{
    return KJSEmbed::g_ctorHash[className.qstring()]->methods;
}

const Constructor *StaticConstructor::constructor( const KJS::UString &className )
{
    return KJSEmbed::g_ctorHash[className.qstring()];
}

KJS::JSObject *StaticConstructor::construct( KJS::ExecState *exec, KJS::JSObject *parent, const KJS::UString &className, const KJS::List &args )
{
    if( parent->hasProperty( exec, className.ascii() ) )
    {
        KJS::JSObject *ctor = parent->get(exec,className.ascii())->toObject(exec);
        if( ctor )
            return ctor->construct( exec, args );
    }
//    return throwError( exec, QString("Cannot create %1 objects from javascript.").arg(className.qstring())); // NOTE: fix
    return KJS::throwError( exec, KJS::GeneralError, QString("Cannot create %1 objects from javascript.").arg(className.qstring()) );
}
