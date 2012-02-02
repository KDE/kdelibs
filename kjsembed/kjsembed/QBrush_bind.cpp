#include "QBrush_bind.h"
#include <QBrush>
#include <value_binding.h>
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
//  style
KJS::JSValue *style( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 0 )
    {
        Qt::BrushStyle tmp = value.style();
        result = KJS::jsNumber( tmp );
        imp->setValue(qVariantFromValue(value)); 
        return result; 
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.style"); 
}

//  setStyle
KJS::JSValue *setStyle( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 1 )
    {
        KJS::JSValue* value0=args[0];
        KJS::JSObject* object0=value0->toObject(exec);
        if(object0 && object0->isNumber())
        {
            Qt::BrushStyle arg0 = KJSEmbed::extractInteger<Qt::BrushStyle>(exec, args, 0);
            value.setStyle(arg0);
            imp->setValue(qVariantFromValue(value)); 
            return result; 
        }
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.setStyle"); 
}

//  texture
KJS::JSValue *texture( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 0 )
    {
        QPixmap tmp = value.texture();
        result = KJSEmbed::createVariant( exec, "QPixmap", tmp );
        imp->setValue(qVariantFromValue(value)); 
        return result; 
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.texture"); 
}

//  setTexture
KJS::JSValue *setTexture( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 1 )
    {
        KJS::JSValue* value0=args[0];
        KJS::JSObject* object0=value0->toObject(exec);
        if(object0 && object0->inherits(&QPixmapBinding::info))
        {
            QPixmap pixmap = KJSEmbed::extractVariant<QPixmap>(exec, args, 0);
            value.setTexture(pixmap);
            imp->setValue(qVariantFromValue(value)); 
            return result; 
        }
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.setTexture"); 
}

//  color
KJS::JSValue *color( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 0 )
    {
        const QColor & tmp = value.color();
        result = KJSEmbed::createVariant( exec, "QColor", tmp );
        imp->setValue(qVariantFromValue(value)); 
        return result; 
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.color"); 
}

//  setColor
KJS::JSValue *setColor( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 1 )
    {
        KJS::JSValue* value0=args[0];
        KJS::JSObject* object0=value0->toObject(exec);
        if(object0 && object0->inherits(&QColorBinding::info))
        {
            QColor color = KJSEmbed::extractVariant<QColor>(exec, args, 0);
            value.setColor(color);
            imp->setValue(qVariantFromValue(value)); 
            return result; 
        }
        if(object0 && object0->isNumber())
        {
            Qt::GlobalColor color = KJSEmbed::extractInteger<Qt::GlobalColor>(exec, args, 0);
            value.setColor(color);
            imp->setValue(qVariantFromValue(value)); 
            return result; 
        }
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.setColor"); 
}

//  gradient
KJS::JSValue *gradient( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 0 )
    {
        const QGradient * tmp = value.gradient();
        result = KJSEmbed::createValue( exec, "const QGradient *", tmp );
        imp->setValue(qVariantFromValue(value)); 
        return result; 
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.gradient"); 
}

//  isOpaque
KJS::JSValue *isOpaque( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) 
{ 
    Q_UNUSED(args); 
    KJS::JSValue *result = KJS::jsNull(); 
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); 
    if( !imp ) 
        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");

    QBrush value = imp->value<QBrush>();
    if (args.size() == 0 )
    {
        bool tmp = value.isOpaque();
        result = KJSEmbed::createVariant( exec, "bool", tmp );
        imp->setValue(qVariantFromValue(value)); 
        return result; 
    }

    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for QBrush.isOpaque"); 
}

}

const Enumerator KJSEmbed::QBrushData::p_enums[] = {{0, 0 }};

NO_STATICS( KJSEmbed::QBrushData )
const Constructor KJSEmbed::QBrushData::p_constructor = 
{"QBrush", 0, KJS::DontDelete|KJS::ReadOnly, 0, &QBrushData::ctorMethod, p_statics, p_enums, KJSEmbed::QBrushData::p_methods };
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
            QGradient gradient = KJSEmbed::extractValue<QGradient>(exec, args, 0);
            return new KJSEmbed::QBrushBinding(exec, QBrush(gradient));
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
    { "gradient", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::gradient },
    { "isOpaque", 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::isOpaque },
    {0, 0, 0, 0 }
};

