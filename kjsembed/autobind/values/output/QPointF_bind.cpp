<QtGui>

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

// bool isNull() const 
START_VARIANT_METHOD( isNull, QPointF )
   bool tmp = value.isNull();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// qreal x() const 
START_VARIANT_METHOD( x, QPointF )
   qreal tmp = value.x();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal y() const 
START_VARIANT_METHOD( y, QPointF )
   qreal tmp = value.y();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setX(qreal x)
START_VARIANT_METHOD( setX, QPointF )
   qreal x = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// void setY(qreal y)
START_VARIANT_METHOD( setY, QPointF )
   qreal y = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// qreal & rx()
START_VARIANT_METHOD( rx, QPointF )
   qreal & tmp = value.rx();
   result = KJSEmbed::createValue( exec, "qreal &", tmp );
END_VARIANT_METHOD

// qreal & ry()
START_VARIANT_METHOD( ry, QPointF )
   qreal & tmp = value.ry();
   result = KJSEmbed::createValue( exec, "qreal &", tmp );
END_VARIANT_METHOD

// QPoint toPoint() const 
START_VARIANT_METHOD( toPoint, QPointF )
   QPoint tmp = value.toPoint();
   result = KJSEmbed::createValue( exec, "QPoint", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QPointF ) 
NO_STATICS( QPointF ) 



START_CTOR( QPointF, QPointF, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QPointFBinding(exec, QPointF())
   }
   if (args.size() == 1 )
   {
       const QPoint & p = (KJSEmbed::extractObject<const QPoint &>(exec, args, 0, 0));
       return new KJSEmbed::QPointFBinding(exec, QPointF(p))
   }
   if (args.size() == 2 )
   {
       qreal xpos = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
       qreal ypos = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
       return new KJSEmbed::QPointFBinding(exec, QPointF(xpos, ypos))
   }
END_CTOR


START_METHOD_LUT( QPointF )
    { isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::isNull },
    { x, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::x },
    { y, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::y },
    { setX, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::setX },
    { setY, 1, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::setY },
    { rx, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::rx },
    { ry, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::ry },
    { toPoint, 0, KJS::DontDelete|KJS::ReadOnly, &QPointFNS::toPoint },
END_METHOD_LUT

