#include "object_binding.h"
#include "value_binding.h"

#include "static_binding.h"
#include <qobjectdefs.h>
#include <QDebug>

using namespace KJSEmbed;

QGenericArgument ObjectBinding::arg(const char *type) const
{
    void *p = voidStar();
//    qDebug("Value ptr %0x", p );
    qDebug() << p;

    return QGenericArgument( type, &p );
}


ObjectBinding::~ObjectBinding()
{
    if( m_owner == JSOwned )
    {
        m_value->cleanup();
    }

    delete m_value;
}

const char *ObjectBinding::typeName() const
{
    return m_name;
}

KJS::UString ObjectBinding::toString(KJS::ExecState */*exec*/) const
{
    return KJS::UString( typeName() );
}

KJS::UString ObjectBinding::className() const
{
    return KJS::UString( typeName() );
}

KJS::Type ObjectBinding::type() const
{
    return KJS::ObjectType;
}

ObjectBinding::Ownership ObjectBinding::ownership() const
{
    return m_owner;
}

void ObjectBinding::setOwnership( ObjectBinding::Ownership owner )
{
    m_owner = owner;
}

KJS::JSValue *callPointerName( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &/*args*/ )
{
    KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec,  self );
    if( imp )
    {
        return KJS::String( imp->typeName() );
    }
    return KJS::Null();
}

KJS::JSValue *callPointerCast( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &/*args*/ )
{
    KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec,  self );
    if( imp )
    {
        return KJS::Boolean(false);
    }
    return KJS::Null();
}

KJS::JSValue *callPointerToString( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &/*args*/ )
{
    KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec,  self );
    if( imp )
    {
        qDebug("Object to string");
        return KJS::String( imp->typeName() );
    }
    return KJS::Null();
}

const Method ObjectFactory::ObjectMethods[] =
{
    {"type", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerName },
    {"cast", 1, KJS::DontDelete|KJS::ReadOnly, &callPointerCast },
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerToString },
    {0, 0, 0, 0 }
};
