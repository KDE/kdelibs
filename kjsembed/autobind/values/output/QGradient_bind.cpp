#include <QDebug>


#include <QGradient_bind.h>

using namespace KJSEmbed;

QGradientBinding::QGradient( KJS::ExecState *exec, const QGradient &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QGradient::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QGradientNS
{

START_CTOR(QGradient, QGradient, 0 )
   return new KJSEmbed::QGradientBinding(exec, QGradient())
END_CTOR

// Type type() const 
START_VARIANT_METHOD( type, QGradient )
END_VARIANT_METHOD

// void setSpread(Spread spread)
START_VARIANT_METHOD( setSpread, QGradient )
   Spread spread = KJSEmbed::extractObject<Spread>(exec, args, 0, 0);
END_VARIANT_METHOD

// Spread spread() const 
START_VARIANT_METHOD( spread, QGradient )
END_VARIANT_METHOD

// void setColorAt(qreal pos, const QColor &color)
START_VARIANT_METHOD( setColorAt, QGradient )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 1, 0);
END_VARIANT_METHOD

// void setStops(const QGradientStops &stops)
START_VARIANT_METHOD( setStops, QGradient )
   const QGradientStops & stops = KJSEmbed::extractObject<const QGradientStops &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QGradientStops stops() const 
START_VARIANT_METHOD( stops, QGradient )
END_VARIANT_METHOD
}


START_METHOD_LUT( QGradient)
{QGradient, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::QGradient },
{type, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::type },
{setSpread, 1, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::setSpread },
{spread, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::spread },
{setColorAt, 2, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::setColorAt },
{setStops, 1, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::setStops },
{stops, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::stops },
{operator==, 1, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::operator== },
{operator==, 1, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::operator== },
END_METHOD_LUT

