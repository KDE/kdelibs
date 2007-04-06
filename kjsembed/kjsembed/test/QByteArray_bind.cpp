#include "QByteArray_bind.h"
#include <QtCore/QByteArray>
#include <variant_binding.h>
#include <value_binding.h>


using namespace KJSEmbed;

const KJS::ClassInfo QByteArrayBinding::info = { "QByteArray", 0, 0, 0 };
QByteArrayBinding::QByteArrayBinding( KJS::ExecState *exec, const QByteArray &value )
   : VariantBinding(exec, value)
{
    StaticBinding::publish(exec, this, QByteArrayData::methods() );
    StaticBinding::publish(exec, this, VariantFactory::methods() );
}


namespace QByteArrayNS
{
}

const Enumerator KJSEmbed::QByteArrayData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QByteArrayData )
const Constructor KJSEmbed::QByteArrayData::p_constructor = 
{"QByteArray", 0, KJS::DontDelete|KJS::ReadOnly, &QByteArrayData::ctorMethod, p_statics, p_enums, KJSEmbed::QByteArrayData::p_methods };
KJS::JSObject *KJSEmbed::QByteArrayData::ctorMethod( KJS::ExecState *exec, const KJS::List &args )
{
    if (args.size() == 0 )
        return new KJSEmbed::QByteArrayBinding(exec, QByteArray());

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QByteArray");
}

const Method KJSEmbed::QByteArrayData::p_methods[] = 
{
    {0, 0, 0, 0 }
};

