#include "QPaintDevice_bind.h"
#include <qPaintDevice.h>
#include <object_binding.h>
#include <value_binding.h>


using namespace KJSEmbed;

const KJS::ClassInfo QPaintDeviceBinding::info = { "QPaintDevice", 0, 0, 0 };
QPaintDeviceBinding::QPaintDeviceBinding( KJS::ExecState *exec, QPaintDevice *value )
   : ObjectBinding<QPaintDevice>(exec, value)
{
    StaticBinding::publish(exec, this, QPaintDeviceData::methods() );
    StaticBinding::publish(exec, this, VariantFactory::methods() );
}


namespace QPaintDeviceNS
{
}

const Enumerator KJSEmbed::QPaintDeviceData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QPaintDeviceData )
const Constructor KJSEmbed::QPaintDeviceData::p_constructor = 
{"QPaintDevice", 0, KJS::DontDelete|KJS::ReadOnly, &QPaintDeviceData::ctorMethod, p_statics, p_enums, KJSEmbed::QPaintDeviceData::p_methods };
KJS::JSObject *KJSEmbed::QPaintDeviceData::ctorMethod( KJS::ExecState *exec, const KJS::List &args )
{
    if (args.size() == 0 )
        return new KJSEmbed::QPaintDeviceBinding(exec, new QPaintDevice);

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QPaintDevice");
}

const Method KJSEmbed::QPaintDeviceData::p_methods[] = 
{
    {0, 0, 0, 0 }
};

