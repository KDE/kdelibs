#include <QDebug>


#include <QRadialGradient_bind.h>

using namespace KJSEmbed;

QRadialGradientBinding::QRadialGradient( KJS::ExecState *exec, const QRadialGradient &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QRadialGradient::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QRadialGradientNS
{

START_CTOR(QRadialGradient, QRadialGradient, 3 )
   const QPointF & center = KJSEmbed::extractObject<const QPointF &>(exec, args, 0, 0);
   qreal radius = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   const QPointF & focalPoint = KJSEmbed::extractObject<const QPointF &>(exec, args, 2, QPointF());
   return new KJSEmbed::QRadialGradientBinding(exec, QRadialGradient(center, radius, focalPoint))
END_CTOR

START_CTOR(QRadialGradient, QRadialGradient, 5 )
   qreal cx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal cy = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal radius = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   qreal fx = KJSEmbed::extractObject<qreal>(exec, args, 3, 0);
   qreal fy = KJSEmbed::extractObject<qreal>(exec, args, 4, 0);
   return new KJSEmbed::QRadialGradientBinding(exec, QRadialGradient(cx, cy, radius, fx, fy))
END_CTOR

// QPointF center() const 
START_VARIANT_METHOD( center, QRadialGradient )
END_VARIANT_METHOD

// QPointF focalPoint() const 
START_VARIANT_METHOD( focalPoint, QRadialGradient )
END_VARIANT_METHOD

// qreal radius() const 
START_VARIANT_METHOD( radius, QRadialGradient )
END_VARIANT_METHOD
}


START_METHOD_LUT( QRadialGradient)
{QRadialGradient, 3, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::QRadialGradient },
{QRadialGradient, 5, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::QRadialGradient },
{center, 0, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::center },
{focalPoint, 0, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::focalPoint },
{radius, 0, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::radius },
END_METHOD_LUT

