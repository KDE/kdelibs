#include "QIODevice_bind.h"
#include <qIODevice.h>
#include <object_binding.h>
#include <value_binding.h>


using namespace KJSEmbed;

const KJS::ClassInfo QIODeviceBinding::info = { "QIODevice", 0, 0, 0 };
QIODeviceBinding::QIODeviceBinding( KJS::ExecState *exec, QIODevice *value )
   : ObjectBinding<QIODevice>(exec, value)
{
    StaticBinding::publish(exec, this, QIODeviceData::methods() );
    StaticBinding::publish(exec, this, VariantFactory::methods() );
}


namespace QIODeviceNS
{
}

const Enumerator KJSEmbed::QIODeviceData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QIODeviceData )
const Constructor KJSEmbed::QIODeviceData::p_constructor = 
{"QIODevice", 0, KJS::DontDelete|KJS::ReadOnly, &QIODeviceData::ctorMethod, p_statics, p_enums, KJSEmbed::QIODeviceData::p_methods };
KJS::JSObject *KJSEmbed::QIODeviceData::ctorMethod( KJS::ExecState *exec, const KJS::List &args )
{
    if (args.size() == 0 )
        return new KJSEmbed::QIODeviceBinding(exec, new QIODevice);

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QIODevice");
}

const Method KJSEmbed::QIODeviceData::p_methods[] = 
{
    {0, 0, 0, 0 }
};

