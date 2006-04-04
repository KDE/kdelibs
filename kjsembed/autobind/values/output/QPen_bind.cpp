<QtGui>

#include <QPen_bind.h>

using namespace KJSEmbed;

QPenBinding::QPen( KJS::ExecState *exec, const QPen &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QPen::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QPenNS
{

// Qt::PenStyle style() const 
START_VARIANT_METHOD( style, QPen )
   Qt::PenStyle tmp = value.style();
   result = KJS::Number( tmp );
END_VARIANT_METHOD

// void setStyle(Qt::PenStyle)
START_VARIANT_METHOD( setStyle, QPen )
   Qt::PenStyle  = static_cast<Qt::PenStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
END_VARIANT_METHOD

// QVector< qreal > dashPattern() const 
START_VARIANT_METHOD( dashPattern, QPen )
   QVector< qreal > tmp = value.dashPattern();
   result = KJSEmbed::createValue( exec, "QVector< qreal >", tmp );
END_VARIANT_METHOD

// void setDashPattern(const QVector< qreal > &pattern)
START_VARIANT_METHOD( setDashPattern, QPen )
   const QVector< qreal > & pattern = (KJSEmbed::extractObject<const QVector< qreal > &>(exec, args, 0, 0));
END_VARIANT_METHOD

// qreal miterLimit() const 
START_VARIANT_METHOD( miterLimit, QPen )
   qreal tmp = value.miterLimit();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setMiterLimit(qreal limit)
START_VARIANT_METHOD( setMiterLimit, QPen )
   qreal limit = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// qreal widthF() const 
START_VARIANT_METHOD( widthF, QPen )
   qreal tmp = value.widthF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setWidthF(qreal width)
START_VARIANT_METHOD( setWidthF, QPen )
   qreal width = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// int width() const 
START_VARIANT_METHOD( width, QPen )
   int tmp = value.width();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setWidth(int width)
START_VARIANT_METHOD( setWidth, QPen )
   int width = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
END_VARIANT_METHOD

// QColor color() const 
START_VARIANT_METHOD( color, QPen )
   QColor tmp = value.color();
   result = KJSEmbed::createValue( exec, "QColor", tmp );
END_VARIANT_METHOD

// void setColor(const QColor &color)
START_VARIANT_METHOD( setColor, QPen )
   const QColor & color = (KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0));
END_VARIANT_METHOD

// QBrush brush() const 
START_VARIANT_METHOD( brush, QPen )
   QBrush tmp = value.brush();
   result = KJSEmbed::createValue( exec, "QBrush", tmp );
END_VARIANT_METHOD

// void setBrush(const QBrush &brush)
START_VARIANT_METHOD( setBrush, QPen )
   const QBrush & brush = (KJSEmbed::extractObject<const QBrush &>(exec, args, 0, 0));
END_VARIANT_METHOD

// bool isSolid() const 
START_VARIANT_METHOD( isSolid, QPen )
   bool tmp = value.isSolid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// Qt::PenCapStyle capStyle() const 
START_VARIANT_METHOD( capStyle, QPen )
   Qt::PenCapStyle tmp = value.capStyle();
   result = KJS::Number( tmp );
END_VARIANT_METHOD

// void setCapStyle(Qt::PenCapStyle pcs)
START_VARIANT_METHOD( setCapStyle, QPen )
   Qt::PenCapStyle pcs = static_cast<Qt::PenCapStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
END_VARIANT_METHOD

// Qt::PenJoinStyle joinStyle() const 
START_VARIANT_METHOD( joinStyle, QPen )
   Qt::PenJoinStyle tmp = value.joinStyle();
   result = KJS::Number( tmp );
END_VARIANT_METHOD

// void setJoinStyle(Qt::PenJoinStyle pcs)
START_VARIANT_METHOD( setJoinStyle, QPen )
   Qt::PenJoinStyle pcs = static_cast<Qt::PenJoinStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
END_VARIANT_METHOD

// bool isDetached()
START_VARIANT_METHOD( isDetached, QPen )
   bool tmp = value.isDetached();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QPen ) 
NO_STATICS( QPen ) 



START_CTOR( QPen, QPen, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QPenBinding(exec, QPen())
   }
   if (args.size() == 1 )
   {
       Qt::PenStyle  = static_cast<Qt::PenStyle>(KJSEmbed::extractInt(exec, args, 0, 0));
       return new KJSEmbed::QPenBinding(exec, QPen())
   }
   if (args.size() == 1 )
   {
       const QColor & color = (KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0));
       return new KJSEmbed::QPenBinding(exec, QPen(color))
   }
   if (args.size() == 5 )
   {
       const QBrush & brush = (KJSEmbed::extractObject<const QBrush &>(exec, args, 0, 0));
       qreal width = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
       Qt::PenStyle s = static_cast<Qt::PenStyle>(KJSEmbed::extractInt(exec, args, 2, Qt::SolidLine));
       Qt::PenCapStyle c = static_cast<Qt::PenCapStyle>(KJSEmbed::extractInt(exec, args, 3, Qt::SquareCap));
       Qt::PenJoinStyle j = static_cast<Qt::PenJoinStyle>(KJSEmbed::extractInt(exec, args, 4, Qt::BevelJoin));
       return new KJSEmbed::QPenBinding(exec, QPen(brush, width, s, c, j))
   }
   if (args.size() == 1 )
   {
       const QPen & pen = (KJSEmbed::extractObject<const QPen &>(exec, args, 0, 0));
       return new KJSEmbed::QPenBinding(exec, QPen(pen))
   }
END_CTOR


START_METHOD_LUT( QPen )
    { style, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::style },
    { setStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setStyle },
    { dashPattern, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::dashPattern },
    { setDashPattern, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setDashPattern },
    { miterLimit, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::miterLimit },
    { setMiterLimit, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setMiterLimit },
    { widthF, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::widthF },
    { setWidthF, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setWidthF },
    { width, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::width },
    { setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setWidth },
    { color, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::color },
    { setColor, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setColor },
    { brush, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::brush },
    { setBrush, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setBrush },
    { isSolid, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::isSolid },
    { capStyle, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::capStyle },
    { setCapStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setCapStyle },
    { joinStyle, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::joinStyle },
    { setJoinStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setJoinStyle },
    { isDetached, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::isDetached },
END_METHOD_LUT

