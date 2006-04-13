<QtGui>

#include <QConicalGradient_bind.h>

using namespace KJSEmbed;

const KJS::ClassInfo QConicalGradientBinding::info = { "QConicalGradientBinding", &ValueBinding::info, 0, 0 };
QConicalGradientBinding::QConicalGradient( KJS::ExecState *exec, const QConicalGradient &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QConicalGradient::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QConicalGradientNS
{

// QPointF center() const 
START_VARIANT_METHOD( center, QConicalGradient )
   QPointF tmp = value.center();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// qreal angle() const 
START_VARIANT_METHOD( angle, QConicalGradient )
   qreal tmp = value.angle();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QConicalGradient ) 
NO_STATICS( QConicalGradient ) 



START_CTOR( QConicalGradient, QConicalGradient, 0 )
   if (args.size() == 2 )
   {
   const QPointF & center = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
   qreal startAngle = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
       return new KJSEmbed::QConicalGradientBinding(exec, QConicalGradient(center, startAngle))
   }
   if (args.size() == 3 )
   {
   qreal cx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal cy = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal startAngle = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
       return new KJSEmbed::QConicalGradientBinding(exec, QConicalGradient(cx, cy, startAngle))
   }
END_CTOR


START_METHOD_LUT( QConicalGradient )
    { center, 0, KJS::DontDelete|KJS::ReadOnly, &QConicalGradientNS::center },
    { angle, 0, KJS::DontDelete|KJS::ReadOnly, &QConicalGradientNS::angle },
END_METHOD_LUT

