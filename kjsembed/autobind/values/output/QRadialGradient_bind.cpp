<QtGui>

#include <QRadialGradient_bind.h>

using namespace KJSEmbed;

const KJS::ClassInfo QRadialGradientBinding::info = { "QRadialGradientBinding", &ValueBinding::info, 0, 0 };
QRadialGradientBinding::QRadialGradient( KJS::ExecState *exec, const QRadialGradient &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QRadialGradient::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QRadialGradientNS
{

// QPointF center() const 
START_VARIANT_METHOD( center, QRadialGradient )
   QPointF tmp = value.center();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// QPointF focalPoint() const 
START_VARIANT_METHOD( focalPoint, QRadialGradient )
   QPointF tmp = value.focalPoint();
   result = KJSEmbed::createValue( exec, "QPointF", tmp );
END_VARIANT_METHOD

// qreal radius() const 
START_VARIANT_METHOD( radius, QRadialGradient )
   qreal tmp = value.radius();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QRadialGradient ) 
NO_STATICS( QRadialGradient ) 



START_CTOR( QRadialGradient, QRadialGradient, 0 )
   if (args.size() == 3 )
   {
   const QPointF & center = KJSEmbed::extractValue<const QPointF &>(exec, args, 0);
   qreal radius = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   const QPointF & focalPoint = KJSEmbed::extractValue<const QPointF &>(exec, args, QPointF());
       return new KJSEmbed::QRadialGradientBinding(exec, QRadialGradient(center, radius, focalPoint))
   }
   if (args.size() == 5 )
   {
   qreal cx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal cy = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal radius = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal fx = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal fy = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
       return new KJSEmbed::QRadialGradientBinding(exec, QRadialGradient(cx, cy, radius, fx, fy))
   }
END_CTOR


START_METHOD_LUT( QRadialGradient )
    { center, 0, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::center },
    { focalPoint, 0, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::focalPoint },
    { radius, 0, KJS::DontDelete|KJS::ReadOnly, &QRadialGradientNS::radius },
END_METHOD_LUT

