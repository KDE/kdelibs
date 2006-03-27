#include <QDebug>


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

START_CTOR(QPen, QPen, 0 )
   return new KJSEmbed::QPenBinding(exec, QPen())
END_CTOR

START_CTOR(QPen, QPen, 1 )
   Qt::PenStyle  = (Qt::PenStyle)KJSEmbed::extractInt(exec, args, 0, 0);
   return new KJSEmbed::QPenBinding(exec, QPen())
END_CTOR

START_CTOR(QPen, QPen, 1 )
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0);
   return new KJSEmbed::QPenBinding(exec, QPen(color))
END_CTOR

START_CTOR(QPen, QPen, 5 )
   const QBrush & brush = KJSEmbed::extractObject<const QBrush &>(exec, args, 0, 0);
   qreal width = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   Qt::PenStyle s = (Qt::PenStyle)KJSEmbed::extractInt(exec, args, 2, Qt::SolidLine);
   Qt::PenCapStyle c = (Qt::PenCapStyle)KJSEmbed::extractInt(exec, args, 3, Qt::SquareCap);
   Qt::PenJoinStyle j = (Qt::PenJoinStyle)KJSEmbed::extractInt(exec, args, 4, Qt::BevelJoin);
   return new KJSEmbed::QPenBinding(exec, QPen(brush, width, s, c, j))
END_CTOR

START_CTOR(QPen, QPen, 1 )
   const QPen & pen = KJSEmbed::extractObject<const QPen &>(exec, args, 0, 0);
   return new KJSEmbed::QPenBinding(exec, QPen(pen))
END_CTOR

START_CTOR(QPen, QPen, 0 )
   return new KJSEmbed::QPenBinding(exec, QPen())
END_CTOR

// Qt::PenStyle style() const 
START_VARIANT_METHOD( style, QPen )
END_VARIANT_METHOD

// void setStyle(Qt::PenStyle)
START_VARIANT_METHOD( setStyle, QPen )
   Qt::PenStyle  = (Qt::PenStyle)KJSEmbed::extractInt(exec, args, 0, 0);
END_VARIANT_METHOD

// QVector< qreal > dashPattern() const 
START_VARIANT_METHOD( dashPattern, QPen )
END_VARIANT_METHOD

// void setDashPattern(const QVector< qreal > &pattern)
START_VARIANT_METHOD( setDashPattern, QPen )
   const QVector< qreal > & pattern = KJSEmbed::extractObject<const QVector< qreal > &>(exec, args, 0, 0);
END_VARIANT_METHOD

// qreal miterLimit() const 
START_VARIANT_METHOD( miterLimit, QPen )
END_VARIANT_METHOD

// void setMiterLimit(qreal limit)
START_VARIANT_METHOD( setMiterLimit, QPen )
   qreal limit = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// qreal widthF() const 
START_VARIANT_METHOD( widthF, QPen )
END_VARIANT_METHOD

// void setWidthF(qreal width)
START_VARIANT_METHOD( setWidthF, QPen )
   qreal width = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// int width() const 
START_VARIANT_METHOD( width, QPen )
END_VARIANT_METHOD

// void setWidth(int width)
START_VARIANT_METHOD( setWidth, QPen )
   int width = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// QColor color() const 
START_VARIANT_METHOD( color, QPen )
END_VARIANT_METHOD

// void setColor(const QColor &color)
START_VARIANT_METHOD( setColor, QPen )
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QBrush brush() const 
START_VARIANT_METHOD( brush, QPen )
END_VARIANT_METHOD

// void setBrush(const QBrush &brush)
START_VARIANT_METHOD( setBrush, QPen )
   const QBrush & brush = KJSEmbed::extractObject<const QBrush &>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool isSolid() const 
START_VARIANT_METHOD( isSolid, QPen )
END_VARIANT_METHOD

// Qt::PenCapStyle capStyle() const 
START_VARIANT_METHOD( capStyle, QPen )
END_VARIANT_METHOD

// void setCapStyle(Qt::PenCapStyle pcs)
START_VARIANT_METHOD( setCapStyle, QPen )
   Qt::PenCapStyle pcs = (Qt::PenCapStyle)KJSEmbed::extractInt(exec, args, 0, 0);
END_VARIANT_METHOD

// Qt::PenJoinStyle joinStyle() const 
START_VARIANT_METHOD( joinStyle, QPen )
END_VARIANT_METHOD

// void setJoinStyle(Qt::PenJoinStyle pcs)
START_VARIANT_METHOD( setJoinStyle, QPen )
   Qt::PenJoinStyle pcs = (Qt::PenJoinStyle)KJSEmbed::extractInt(exec, args, 0, 0);
END_VARIANT_METHOD

// bool isDetached()
START_VARIANT_METHOD( isDetached, QPen )
END_VARIANT_METHOD

// void detach()
START_VARIANT_METHOD( detach, QPen )
END_VARIANT_METHOD
}


START_METHOD_LUT( QPen)
{QPen, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::QPen },
{QPen, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::QPen },
{QPen, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::QPen },
{QPen, 5, KJS::DontDelete|KJS::ReadOnly, &QPenNS::QPen },
{QPen, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::QPen },
{~QPen, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::~QPen },
{operator=, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::operator= },
{style, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::style },
{setStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setStyle },
{dashPattern, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::dashPattern },
{setDashPattern, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setDashPattern },
{miterLimit, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::miterLimit },
{setMiterLimit, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setMiterLimit },
{widthF, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::widthF },
{setWidthF, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setWidthF },
{width, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::width },
{setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setWidth },
{color, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::color },
{setColor, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setColor },
{brush, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::brush },
{setBrush, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setBrush },
{isSolid, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::isSolid },
{capStyle, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::capStyle },
{setCapStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setCapStyle },
{joinStyle, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::joinStyle },
{setJoinStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::setJoinStyle },
{operator==, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::operator== },
{operator!=, 1, KJS::DontDelete|KJS::ReadOnly, &QPenNS::operator!= },
{operator QVariant, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::operator QVariant },
{isDetached, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::isDetached },
{detach, 0, KJS::DontDelete|KJS::ReadOnly, &QPenNS::detach },
END_METHOD_LUT

