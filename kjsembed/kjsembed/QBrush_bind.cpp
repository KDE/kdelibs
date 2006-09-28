#include <qbrush.h>
#include <object_binding.h>

#include "QBrush_bind.h"

using namespace KJSEmbed;

// Temp - for building
class QColorBinding { public: static const KJS::ClassInfo info; };
class QPixmapBinding { public: static const KJS::ClassInfo info; };
class QGradientBinding { public: static const KJS::ClassInfo info; };

const KJS::ClassInfo QColorBinding::info = { "QColor", &VariantBinding::info, 0, 0 };
const KJS::ClassInfo QPixmapBinding::info = { "QPixmap", &VariantBinding::info, 0, 0 };
const KJS::ClassInfo QGradientBinding::info = { "QGradient", &VariantBinding::info, 0, 0 };

const KJS::ClassInfo QBrushBinding::info = { "QBrush", &VariantBinding::info, 0, 0 };

QBrushBinding::QBrushBinding( KJS::ExecState *exec, const QBrush &value )
    : VariantBinding(exec, value)
{
    StaticBinding::publish(exec, this, QBrushData::methods() );
    StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QBrushNS
{

// Qt::BrushStyle style() const 
KJS::JSValue *style( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        Qt::BrushStyle tmp = value.style();
        result = KJS::Number( tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result;
}

// void setStyle(Qt::BrushStyle)
KJS::JSValue *setStyle( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        Qt::BrushStyle arg0 = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
        value.setStyle(arg0);
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result;
}

// QPixmap texture() const 
KJS::JSValue *texture( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        QPixmap tmp = value.texture();
       result = KJSEmbed::createValue( exec, "QPixmap", tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result; 
}

// void setTexture(const QPixmap &pixmap)
KJS::JSValue *setTexture( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        QPixmap pixmap = KJSEmbed::extractValue<QPixmap>(exec, args, 0);
        value.setTexture(pixmap);
        imp->setValue(qVariantFromValue(value)); 
    }
    else
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result;
}

// const QColor & color() const 
KJS::JSValue *color( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        const QColor & tmp = value.color();
        result = KJSEmbed::createValue( exec, "const QColor &", tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result;
}

KJS::JSValue *setColor( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp )
    {
        QBrush value = imp->value<QBrush>();

        if (args.size() == 1 )
        {
            KJS::JSValue* value0=args[0];
            if (value0->isNumber())
            {
                Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
                value.setColor(color);
            }
            else
            {
                QColor color = KJSEmbed::extractValue<QColor>(exec, args, 0);
                value.setColor(color);
            }
        }

        imp->setValue(qVariantFromValue(value)); 
    }
    else
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result;
}

// const QGradient * gradient() const 
/*
KJS::JSValue *gradient( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null();
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self);
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        const QGradient *tmp = value.gradient();
        result = KJSEmbed::createValue(exec, "QGradient", tmp);
        imp->setValue(qVariantFromValue(value)); 
    }
    else
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");

    return result;
}
*/

// bool isOpaque() const 
KJS::JSValue *isOpaque( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self);
    if( imp )
    {
        QBrush value = imp->value<QBrush>();
        bool tmp = value.isOpaque();
        result = KJSEmbed::createValue( exec, "bool", tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result;
}

}

const Enumerator KJSEmbed::QBrushData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QBrushData )
const Constructor KJSEmbed::QBrushData::p_constructor = 
{"QBrush", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushData::ctorMethod, p_statics, p_enums, p_methods };
KJS::JSObject *KJSEmbed::QBrushData::ctorMethod( KJS::ExecState *exec, const KJS::List &args )
{
    if (args.size() == 0 )
    {
        return new KJSEmbed::QBrushBinding(exec, QBrush());
    }
    if (args.size() == 1 )
    {
        KJS::JSValue* value0=args[0];
        KJS::JSObject* object0=value0->toObject(exec);
        if(value0->isNumber())
        {
            Qt::BrushStyle bs = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
            return new KJSEmbed::QBrushBinding(exec, QBrush(bs));
        }
        if(object0->inherits(&QPixmapBinding::info))
        {
            QPixmap pixmap = KJSEmbed::extractValue<QPixmap>(exec, args, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(pixmap));
        }
        if(object0->inherits(&QBrushBinding::info))
        {
            QBrush brush = KJSEmbed::extractValue<QBrush>(exec, args, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(brush));
        }
//        if(object0->inherits(&QGradientBinding::info))
//       {
//            QGradient gradient = KJSEmbed::extractValue<QGradient>(exec, args, 0);
//            return new KJSEmbed::QBrushBinding(exec, QBrush(gradient));
//        }
    }
    if (args.size() == 2 )
    {
        KJS::JSValue* value0=args[0];
        KJS::JSObject* object0=value0->toObject(exec);
        KJS::JSValue* value1=args[1];
        KJS::JSObject* object1=value1->toObject(exec);

        if(object0->inherits(&QColorBinding::info) && value1->isNumber())
        {
            QColor color = KJSEmbed::extractValue<QColor>(exec, args, 0);
            Qt::BrushStyle bs = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 1, Qt::SolidPattern));
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs));
        }
        if(value0->isNumber() && value1->isNumber())
        {
            Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
            Qt::BrushStyle bs = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 1, Qt::SolidPattern));
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs));
        }
        if(object0->inherits(&QColorBinding::info) && object1->inherits(&QPixmapBinding::info))
        {
            QColor color = KJSEmbed::extractValue<QColor>(exec, args, 0);
            QPixmap pixmap = KJSEmbed::extractValue<QPixmap>(exec, args, 1);
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap));
        }
        if(value0->isNumber() && object1->inherits(&QPixmapBinding::info))
        {
            Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
            QPixmap pixmap = KJSEmbed::extractValue<QPixmap>(exec, args, 1);
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap));
        }
    }
    return 0;
}

const Method KJSEmbed::QBrushData::p_methods[] =
{
    { "style",      0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::style },
    { "setStyle",   1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setStyle },
    { "texture",    0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::texture },
    { "setTexture", 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setTexture },
    { "color",      0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::color },
    { "setColor",   1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
    { "setColor",   1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
//    { "gradient",   0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::gradient },
    { "isOpaque",   0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::isOpaque },
    { 0, 0, 0, 0 }
};

