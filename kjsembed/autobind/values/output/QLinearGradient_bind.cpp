<QtGui>

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

// QPointF start() const 
START_VARIANT_METHOD( start, QLinearGradient )
   QPointF tmp = value.start();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// QPointF finalStop() const 
START_VARIANT_METHOD( finalStop, QLinearGradient )
   QPointF tmp = value.finalStop();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QLinearGradient ) 
NO_STATICS( QLinearGradient ) 



START_CTOR( QLinearGradient, QLinearGradient, 0 )
   if (args.size() == 2 )
   {
   const QPointF & start = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
   const QPointF & finalStop = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
       return new KJSEmbed::QLinearGradientBinding(exec, QLinearGradient(start, finalStop))
   }
   if (args.size() == 4 )
   {
   qreal xStart = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal yStart = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal xFinalStop = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal yFinalStop = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
       return new KJSEmbed::QLinearGradientBinding(exec, QLinearGradient(xStart, yStart, xFinalStop, yFinalStop))
   }
END_CTOR


START_METHOD_LUT( QLinearGradient )
    { start, 0, KJS::DontDelete|KJS::ReadOnly, &QLinearGradientNS::start },
    { finalStop, 0, KJS::DontDelete|KJS::ReadOnly, &QLinearGradientNS::finalStop },
END_METHOD_LUT

