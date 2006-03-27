#include <QDebug>


#include <QPointF_bind.h>

using namespace KJSEmbed;

QPointFBinding::QPointF( KJS::ExecState *exec, const QPointF &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QPointF::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QPointFNS
{

START_CTOR(QPointF, QPointF, 0 )
   return new KJSEmbed::QPointFBinding(exec, QPointF())
END_CTOR

START_CTOR(QPointF, QPointF, 1 )
   const QPoint & p = KJSEmbed::extractObject<const QPoint &>(exec, args, 0, 0);
   return new KJSEmbed::QPointFBinding(exec, QPointF(p))
END_CTOR

START_CTOR(QPointF, QPointF, 2 )
   qreal xpos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal ypos = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   return new KJSEmbed::QPointFBinding(exec, QPointF(xpos, ypos))
END_CTOR

// bool isNull() const 
START_VARIANT_METHOD( isNull, QPointF )
END_VARIANT_METHOD

// qreal x() const 
START_VARIANT_METHOD( x, QPointF )
END_VARIANT_METHOD

// qreal y() const 
START_VARIANT_METHOD( y, QPointF )
END_VARIANT_METHOD

// void setX(qreal x)
START_VARIANT_METHOD( setX, QPointF )
   qreal x = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setY(qreal y)
START_VARIANT_METHOD( setY, QPointF )
   qreal y = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// qreal & rx()
START_VARIANT_METHOD( rx, QPointF )
END_VARIANT_METHOD

// qreal & ry()
START_VARIANT_METHOD( ry, QPointF )
END_VARIANT_METHOD

// QPoint toPoint() const 
START_VARIANT_METHOD( toPoint, QPointF )
END_VARIANT_METHOD
}


START_METHOD_LUT( QPointF)
{QPointF, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::QPointF },
{QPointF, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::QPointF },
{QPointF, 2, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::QPointF },
{isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::isNull },
{x, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::x },
{y, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::y },
{setX, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::setX },
{setY, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::setY },
{rx, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::rx },
{ry, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::ry },
{operator+=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::operator+= },
{operator-=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::operator-= },
{operator *=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::operator *= },
{operator/=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::operator/= },
{toPoint, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::toPoint },
END_METHOD_LUT

