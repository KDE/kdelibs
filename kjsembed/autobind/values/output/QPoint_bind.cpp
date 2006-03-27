#include <QDebug>


#include <QPoint_bind.h>

using namespace KJSEmbed;

QPointBinding::QPoint( KJS::ExecState *exec, const QPoint &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QPoint::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QPointNS
{

START_CTOR(QPoint, QPoint, 0 )
   return new KJSEmbed::QPointBinding(exec, QPoint())
END_CTOR

START_CTOR(QPoint, QPoint, 2 )
   int xpos = KJSEmbed::extractObject<int>(exec, args, 0, 0);
   int ypos = KJSEmbed::extractObject<int>(exec, args, 1, 0);
   return new KJSEmbed::QPointBinding(exec, QPoint(xpos, ypos))
END_CTOR

// bool isNull() const 
START_VARIANT_METHOD( isNull, QPoint )
END_VARIANT_METHOD

// int x() const 
START_VARIANT_METHOD( x, QPoint )
END_VARIANT_METHOD

// int y() const 
START_VARIANT_METHOD( y, QPoint )
END_VARIANT_METHOD

// void setX(int x)
START_VARIANT_METHOD( setX, QPoint )
   int x = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setY(int y)
START_VARIANT_METHOD( setY, QPoint )
   int y = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// int manhattanLength() const 
START_VARIANT_METHOD( manhattanLength, QPoint )
END_VARIANT_METHOD

// int & rx()
START_VARIANT_METHOD( rx, QPoint )
END_VARIANT_METHOD

// int & ry()
START_VARIANT_METHOD( ry, QPoint )
END_VARIANT_METHOD
}


START_METHOD_LUT( QPoint)
{QPoint, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::QPoint },
{QPoint, 2, KJS::DontDelete|KJS::ReadOnly, &QPointNS::QPoint },
{isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::isNull },
{x, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::x },
{y, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::y },
{setX, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::setX },
{setY, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::setY },
{manhattanLength, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::manhattanLength },
{rx, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::rx },
{ry, 0, KJS::DontDelete|KJS::ReadOnly, &QPointNS::ry },
{operator+=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::operator+= },
{operator-=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::operator-= },
{operator *=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::operator *= },
{operator/=, 1, KJS::DontDelete|KJS::ReadOnly, &QPointNS::operator/= },
END_METHOD_LUT

