#include "QBrush_bind.h"
#include <qbrush.h>
#include <object_binding.h>

using namespace KJSEmbed;

// Temp - for building
class QColorBinding { public: static const KJS::ClassInfo info; };
class QPixmapBinding { public: static const KJS::ClassInfo info; };
class QGradientBinding { public: static const KJS::ClassInfo info; };

const KJS::ClassInfo QColorBinding::info = { "QColor", &VariantBinding::info, 0, 0 };
const KJS::ClassInfo QPixmapBinding::info = { "QPixmap", &VariantBinding::info, 0, 0 };
const KJS::ClassInfo QGradientBinding::info = { "QGradient", &ObjectBinding::info, 0, 0 };

const KJS::ClassInfo QBrushBinding::info = { "QBrush", &VariantBinding::info, 0, 0 };
QBrushBinding::QBrushBinding( KJS::ExecState *exec, const QBrush &value )
   : VariantBinding(exec, value)
{
    StaticBinding::publish(exec, this, QBrushData::methods() );
    StaticBinding::publish(exec, this, VariantFactory::methods() );
}


namespace QBrushNS
{
// Qt::BrushStyle style() const 
KJS::JSValue *style( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
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
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

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
        Qt::BrushStyle arg0 = KJSEmbed::extractInteger<Qt::BrushStyle>(exec, args, 0);
        value.setStyle(arg0);
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result; 
}

// QPixmap texture() const 
KJS::JSValue *texture( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp ) 
    { 
        QBrush value = imp->value<QBrush>();
        QPixmap tmp = value.texture();
        result = KJSEmbed::createVariant( exec, "QPixmap", tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

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
        QPixmap pixmap = KJSEmbed::extractVariant<QPixmap>(exec, args, 0);
        value.setTexture(pixmap);
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result; 
}

// const QColor & color() const 
KJS::JSValue *color( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp ) 
    { 
        QBrush value = imp->value<QBrush>();
        const QColor & tmp = value.color();
        result = KJSEmbed::createVariant( exec, "QColor", tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result; 
}
// void setColor(const QColor &color)
KJS::JSValue *setColor( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp ) 
    { 
        if (args.size() == 1 )
        {
            KJS::JSValue* value0=args[0];
            KJS::JSObject* object0=value0->toObject(exec);
            if (object0 && object0->inherits(&QColorBinding::info))
            {
                QBrush value = imp->value<QBrush>();
                QColor color = KJSEmbed::extractVariant<QColor>(exec, args, 0);
                value.setColor(color);
                imp->setValue(qVariantFromValue(value)); 
            }
            else if (value0->isNumber())
            {
                QBrush value = imp->value<QBrush>();
                Qt::GlobalColor color = KJSEmbed::extractInteger<Qt::GlobalColor>(exec, args, 0);
                value.setColor(color);
                imp->setValue(qVariantFromValue(value)); 
            }
            else
                return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.setColor()");
        }
        else
            return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.setColor()");
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result; 
}

#if 0
// void setColor(Qt::GlobalColor color)
KJS::JSValue *setColor( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp ) 
    { 
        QBrush value = imp->value<QBrush>();
        Qt::GlobalColor color = KJSEmbed::extractInteger<Qt::GlobalColor>(exec, args, 0);
        value.setColor(color);
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result; 
}
#endif

// const QGradient * gradient() const 
KJS::JSValue *gradient( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp ) 
    { 
        QBrush value = imp->value<QBrush>();
        const QGradient * tmp = value.gradient();
        result = KJSEmbed::createObject( exec, "const QGradient *", tmp );
        imp->setValue(qVariantFromValue(value)); 
    }
    else 
    {
        KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");
    }

    return result; 
}

// bool isOpaque() const 
KJS::JSValue *isOpaque( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::Null(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( imp ) 
    { 
        QBrush value = imp->value<QBrush>();
        bool tmp = value.isOpaque();
        result = KJSEmbed::createVariant( exec, "bool", tmp );
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
{"QBrush", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushData::ctorMethod, p_statics, p_enums, KJSEmbed::QBrushData::p_methods };
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
        if(object0 && object0->isNumber())
        {
            Qt::BrushStyle bs = KJSEmbed::extractInteger<Qt::BrushStyle>(exec, args, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(bs));
        }
        if(object0 && object0->inherits(&QPixmapBinding::info))
        {
            QPixmap pixmap = KJSEmbed::extractVariant<QPixmap>(exec, args, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(pixmap));
        }
        if(object0 && object0->inherits(&QBrushBinding::info))
        {
            QBrush brush = KJSEmbed::extractVariant<QBrush>(exec, args, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(brush));
        }
        if(object0 && object0->inherits(&QGradientBinding::info))
        {
            QGradient* gradient = KJSEmbed::extractObject<QGradient>(exec, args, 0, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(*gradient));
        }
    }
    if (args.size() == 2 )
    {
        KJS::JSValue* value0=args[0];
        KJS::JSObject* object0=value0->toObject(exec);
        KJS::JSValue* value1=args[1];
        KJS::JSObject* object1=value1->toObject(exec);
        if(object0 && object0->inherits(&QColorBinding::info) && ( ( object1 && object1->isNumber() ) || !object1 ))
        {
            QColor color = KJSEmbed::extractVariant<QColor>(exec, args, 0);
            Qt::BrushStyle bs = KJSEmbed::extractInteger<Qt::BrushStyle>(exec, args, 1, Qt::SolidPattern);
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs));
        }
        if(object0 && object0->isNumber() && ( ( object1 && object1->isNumber() ) || !object1 ))
        {
            Qt::GlobalColor color = KJSEmbed::extractInteger<Qt::GlobalColor>(exec, args, 0);
            Qt::BrushStyle bs = KJSEmbed::extractInteger<Qt::BrushStyle>(exec, args, 1, Qt::SolidPattern);
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs));
        }
        if(object0 && object0->inherits(&QColorBinding::info) && object1 && object1->inherits(&QPixmapBinding::info))
        {
            QColor color = KJSEmbed::extractVariant<QColor>(exec, args, 0);
            QPixmap pixmap = KJSEmbed::extractVariant<QPixmap>(exec, args, 1);
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap));
        }
        if(object0 && object0->isNumber() && object1 && object1->inherits(&QPixmapBinding::info))
        {
            Qt::GlobalColor color = KJSEmbed::extractInteger<Qt::GlobalColor>(exec, args, 0);
            QPixmap pixmap = KJSEmbed::extractVariant<QPixmap>(exec, args, 1);
            return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap));
        }
    }
    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush");
}

const Method KJSEmbed::QBrushData::p_methods[] = 
{
    { "style", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::style },
    { "setStyle", 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setStyle },
    { "texture", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::texture },
    { "setTexture", 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setTexture },
    { "color", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::color },
    { "setColor", 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
    { "setColor", 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
    { "gradient", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::gradient },
    { "isOpaque", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::isOpaque },
    {0, 0, 0, 0 }
};

