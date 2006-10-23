#include "QIconEngine_bind.h"
#include <qIconEngine.h>
#include <object_binding.h>
#include <value_binding.h>


using namespace KJSEmbed;

const KJS::ClassInfo QIconEngineBinding::info = { "QIconEngine", 0, 0, 0 };
QIconEngineBinding::QIconEngineBinding( KJS::ExecState *exec, QIconEngine *value )
   : ObjectBinding<QIconEngine>(exec, value)
{
    StaticBinding::publish(exec, this, QIconEngineData::methods() );
    StaticBinding::publish(exec, this, VariantFactory::methods() );
}


namespace QIconEngineNS
{
}

const Enumerator KJSEmbed::QIconEngineData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QIconEngineData )
const Constructor KJSEmbed::QIconEngineData::p_constructor = 
{"QIconEngine", 0, KJS::DontDelete|KJS::ReadOnly, &QIconEngineData::ctorMethod, p_statics, p_enums, KJSEmbed::QIconEngineData::p_methods };
KJS::JSObject *KJSEmbed::QIconEngineData::ctorMethod( KJS::ExecState *exec, const KJS::List &args )
{
    if (args.size() == 0 )
        return new KJSEmbed::QIconEngineBinding(exec, new QIconEngine);

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QIconEngine");
}

const Method KJSEmbed::QIconEngineData::p_methods[] = 
{
    {0, 0, 0, 0 }
};

