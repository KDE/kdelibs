#include "scalar_binding.h"
#include "static_binding.h"

using namespace KJSEmbed;

KJS::JSValue *callScalarType( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::ScalarBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ScalarBinding>(exec,  self );
    if( imp )
    {
        return KJS::String( imp->className().ascii() );
    }
    return KJS::Null();
}

const Method ScalarFactory::ScalarMethods[] =
{
    {"type", 0, KJS::DontDelete|KJS::ReadOnly, &callScalarType },
    //{"cast", 1, KJS::DontDelete|KJS::ReadOnly, &callPointerCast },
    //{"toString", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerToString },
    {0, 0, 0, 0 }
};

const Method *ScalarFactory::methods()
{
    return ScalarMethods;
}

KJS::UString ScalarBinding::toString(KJS::ExecState *exec) const
{
    return m_value->type().name();
}
