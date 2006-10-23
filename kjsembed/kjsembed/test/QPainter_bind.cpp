#include "QPainter_bind.h"
#include <qPainter.h>
#include <object_binding.h>
#include <value_binding.h>


using namespace KJSEmbed;

const KJS::ClassInfo QPainterBinding::info = { "QPainter", 0, 0, 0 };
QPainterBinding::QPainterBinding( KJS::ExecState *exec, QPainter *value )
   : ObjectBinding<QPainter>(exec, value)
{
    StaticBinding::publish(exec, this, QPainterData::methods() );
    StaticBinding::publish(exec, this, VariantFactory::methods() );
}


namespace QPainterNS
{
}

const Enumerator KJSEmbed::QPainterData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QPainterData )
const Constructor KJSEmbed::QPainterData::p_constructor = 
{"QPainter", 0, KJS::DontDelete|KJS::ReadOnly, &QPainterData::ctorMethod, p_statics, p_enums, KJSEmbed::QPainterData::p_methods };
KJS::JSObject *KJSEmbed::QPainterData::ctorMethod( KJS::ExecState *exec, const KJS::List &args )
{
    if (args.size() == 0 )
        return new KJSEmbed::QPainterBinding(exec, new QPainter);

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QPainter");
}

const Method KJSEmbed::QPainterData::p_methods[] = 
{
    {0, 0, 0, 0 }
};

