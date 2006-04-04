<QtGui>

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

// Type type() const 
START_VARIANT_METHOD( type, QGradient )
   Type tmp = value.type();
   result = KJSEmbed::createValue( exec, "Type", tmp );
END_VARIANT_METHOD

// void setSpread(Spread spread)
START_VARIANT_METHOD( setSpread, QGradient )
   Spread spread = KJSEmbed::extractObject<Spread>(exec, args, 0, 0);
    value.setSpread(spread);
END_VARIANT_METHOD

// Spread spread() const 
START_VARIANT_METHOD( spread, QGradient )
   Spread tmp = value.spread();
   result = KJSEmbed::createValue( exec, "Spread", tmp );
END_VARIANT_METHOD

// void setColorAt(qreal pos, const QColor &color)
START_VARIANT_METHOD( setColorAt, QGradient )
   qreal pos = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   const QColor & color = KJSEmbed::extractValue<const QColor &>(exec, args, 0);
    value.setColorAt(color);
END_VARIANT_METHOD

// void setStops(const QGradientStops &stops)
START_VARIANT_METHOD( setStops, QGradient )
   const QGradientStops & stops = KJSEmbed::extractObject<const QGradientStops &>(exec, args, 0, 0);
    value.setStops(stops);
END_VARIANT_METHOD

// QGradientStops stops() const 
START_VARIANT_METHOD( stops, QGradient )
   QGradientStops tmp = value.stops();
   result = KJSEmbed::createValue( exec, "QGradientStops", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QGradient ) 
NO_STATICS( QGradient ) 



START_CTOR( QGradient, QGradient, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QGradientBinding(exec, QGradient())
   }
END_CTOR


START_METHOD_LUT( QGradient )
    { type, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::type },
    { setSpread, 1, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::setSpread },
    { spread, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::spread },
    { setColorAt, 2, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::setColorAt },
    { setStops, 1, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::setStops },
    { stops, 0, KJS::DontDelete|KJS::ReadOnly, &QGradientNS::stops },
END_METHOD_LUT

