#include <QDebug>


#include <QLinearGradient_bind.h>

using namespace KJSEmbed;

QLinearGradientBinding::QLinearGradient( KJS::ExecState *exec, const QLinearGradient &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QLinearGradient::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QLinearGradientNS
{

START_CTOR(QLinearGradient, QLinearGradient, 2 )
   const QPointF & start = KJSEmbed::extractObject<const QPointF &>(exec, args, 0, 0);
   const QPointF & finalStop = KJSEmbed::extractObject<const QPointF &>(exec, args, 1, 0);
   return new KJSEmbed::QLinearGradientBinding(exec, QLinearGradient(start, finalStop))
END_CTOR

START_CTOR(QLinearGradient, QLinearGradient, 4 )
   qreal xStart = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal yStart = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   qreal xFinalStop = KJSEmbed::extractObject<qreal>(exec, args, 2, 0);
   qreal yFinalStop = KJSEmbed::extractObject<qreal>(exec, args, 3, 0);
   return new KJSEmbed::QLinearGradientBinding(exec, QLinearGradient(xStart, yStart, xFinalStop, yFinalStop))
END_CTOR

// QPointF start() const 
START_VARIANT_METHOD( start, QLinearGradient )
END_VARIANT_METHOD

// QPointF finalStop() const 
START_VARIANT_METHOD( finalStop, QLinearGradient )
END_VARIANT_METHOD
}


START_METHOD_LUT( QLinearGradient)
{QLinearGradient, 2, KJS::DontDelete|KJS::ReadOnly, &QLinearGradientNS::QLinearGradient },
{QLinearGradient, 4, KJS::DontDelete|KJS::ReadOnly, &QLinearGradientNS::QLinearGradient },
{start, 0, KJS::DontDelete|KJS::ReadOnly, &QLinearGradientNS::start },
{finalStop, 0, KJS::DontDelete|KJS::ReadOnly, &QLinearGradientNS::finalStop },
END_METHOD_LUT

