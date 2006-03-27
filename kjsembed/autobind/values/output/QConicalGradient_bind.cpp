#include <QDebug>


#include <QConicalGradient_bind.h>

using namespace KJSEmbed;

QConicalGradientBinding::QConicalGradient( KJS::ExecState *exec, const QConicalGradient &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QConicalGradient::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QConicalGradientNS
{

START_CTOR(QConicalGradient, QConicalGradient, 2 )
   const QPointF & center = KJSEmbed::extractObject<const QPointF &>(exec, args, 0, 0);
   qreal startAngle = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   return new KJSEmbed::QConicalGradientBinding(exec, QConicalGradient(center, startAngle))
END_CTOR

START_CTOR(QConicalGradient, QConicalGradient, 3 )
   qreal cx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal cy = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal startAngle = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   return new KJSEmbed::QConicalGradientBinding(exec, QConicalGradient(cx, cy, startAngle))
END_CTOR

// QPointF center() const 
START_VARIANT_METHOD( center, QConicalGradient )
END_VARIANT_METHOD

// qreal angle() const 
START_VARIANT_METHOD( angle, QConicalGradient )
END_VARIANT_METHOD
}


START_METHOD_LUT( QConicalGradient)
{QConicalGradient, 2, KJS::DontDelete|KJS::ReadOnly, &QConicalGradientNS::QConicalGradient },
{QConicalGradient, 3, KJS::DontDelete|KJS::ReadOnly, &QConicalGradientNS::QConicalGradient },
{center, 0, KJS::DontDelete|KJS::ReadOnly, &QConicalGradientNS::center },
{angle, 0, KJS::DontDelete|KJS::ReadOnly, &QConicalGradientNS::angle },
END_METHOD_LUT

