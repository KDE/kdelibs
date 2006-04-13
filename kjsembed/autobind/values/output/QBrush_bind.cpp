<QtGui>

#include <QBrush_bind.h>

using namespace KJSEmbed;

const KJS::ClassInfo QBrushBinding::info = { "QBrushBinding", &ValueBinding::info, 0, 0 };
QBrushBinding::QBrush( KJS::ExecState *exec, const QBrush &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QBrush::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QBrushNS
{

// Qt::BrushStyle style() const 
START_VARIANT_METHOD( style, QBrush )
   Qt::BrushStyle tmp = value.style();
   result = KJS::Number( tmp );
END_VARIANT_METHOD

// void setStyle(Qt::BrushStyle)
START_VARIANT_METHOD( setStyle, QBrush )
   Qt::BrushStyle arg0 = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
    value.setStyle(arg0);
END_VARIANT_METHOD

// QPixmap texture() const 
START_VARIANT_METHOD( texture, QBrush )
   QPixmap tmp = value.texture();
   result = KJSEmbed::createValue( exec, "QPixmap", tmp );
END_VARIANT_METHOD

// void setTexture(const QPixmap &pixmap)
START_VARIANT_METHOD( setTexture, QBrush )
   const QPixmap & pixmap = KJSEmbed::extractValue<const QPixmap &>(exec, args, 0);
    value.setTexture(pixmap);
END_VARIANT_METHOD

// const QColor & color() const 
START_VARIANT_METHOD( color, QBrush )
   const QColor & tmp = value.color();
   result = KJSEmbed::createValue( exec, "const QColor &", tmp );
END_VARIANT_METHOD

// void setColor(const QColor &color)
START_VARIANT_METHOD( setColor, QBrush )
   const QColor & color = KJSEmbed::extractValue<const QColor &>(exec, args, 0);
    value.setColor(color);
END_VARIANT_METHOD

// void setColor(Qt::GlobalColor color)
START_VARIANT_METHOD( setColor, QBrush )
   Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
    value.setColor(color);
END_VARIANT_METHOD

// const QGradient * gradient() const 
START_VARIANT_METHOD( gradient, QBrush )
   const QGradient * tmp = value.gradient();
   result = KJSEmbed::createValue( exec, "const QGradient *", tmp );
END_VARIANT_METHOD

// bool isOpaque() const 
START_VARIANT_METHOD( isOpaque, QBrush )
   bool tmp = value.isOpaque();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QBrush ) 
NO_STATICS( QBrush ) 



START_CTOR( QBrush, QBrush, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QBrushBinding(exec, QBrush())
   }
   if (args.size() == 1 )
   {
   Qt::BrushStyle bs = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
       return new KJSEmbed::QBrushBinding(exec, QBrush(bs))
   }
   if (args.size() == 2 )
   {
   const QColor & color = KJSEmbed::extractValue<const QColor &>(exec, args, 0);
   Qt::BrushStyle bs = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 0, Qt::SolidPattern));
       return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs))
   }
   if (args.size() == 2 )
   {
   Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
   Qt::BrushStyle bs = static_cast<Qt::BrushStyle>(KJSEmbed::extractInt(exec, args, 0, Qt::SolidPattern));
       return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs))
   }
   if (args.size() == 2 )
   {
   const QColor & color = KJSEmbed::extractValue<const QColor &>(exec, args, 0);
   const QPixmap & pixmap = KJSEmbed::extractValue<const QPixmap &>(exec, args, 0);
       return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap))
   }
   if (args.size() == 2 )
   {
   Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
   const QPixmap & pixmap = KJSEmbed::extractValue<const QPixmap &>(exec, args, 0);
       return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap))
   }
   if (args.size() == 1 )
   {
   const QPixmap & pixmap = KJSEmbed::extractValue<const QPixmap &>(exec, args, 0);
       return new KJSEmbed::QBrushBinding(exec, QBrush(pixmap))
   }
   if (args.size() == 1 )
   {
   const QBrush & brush = KJSEmbed::extractValue<const QBrush &>(exec, args, 0);
       return new KJSEmbed::QBrushBinding(exec, QBrush(brush))
   }
   if (args.size() == 1 )
   {
   const QGradient & gradient = KJSEmbed::extractObject<const QGradient &>(exec, args, 0, 0);
       return new KJSEmbed::QBrushBinding(exec, QBrush(gradient))
   }
END_CTOR


START_METHOD_LUT( QBrush )
    { style, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::style },
    { setStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setStyle },
    { texture, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::texture },
    { setTexture, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setTexture },
    { color, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::color },
    { setColor, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
    { setColor, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
    { gradient, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::gradient },
    { isOpaque, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::isOpaque },
END_METHOD_LUT

