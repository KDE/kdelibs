<QtGui>

#include <QSizeF_bind.h>

using namespace KJSEmbed;

QSizeFBinding::QSizeF( KJS::ExecState *exec, const QSizeF &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QSizeF::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QSizeFNS
{

// bool isNull() const 
START_VARIANT_METHOD( isNull, QSizeF )
   bool tmp = value.isNull();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QSizeF )
   bool tmp = value.isEmpty();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QSizeF )
   bool tmp = value.isValid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// qreal width() const 
START_VARIANT_METHOD( width, QSizeF )
   qreal tmp = value.width();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal height() const 
START_VARIANT_METHOD( height, QSizeF )
   qreal tmp = value.height();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setWidth(qreal w)
START_VARIANT_METHOD( setWidth, QSizeF )
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setWidth(w);
END_VARIANT_METHOD

// void setHeight(qreal h)
START_VARIANT_METHOD( setHeight, QSizeF )
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setHeight(h);
END_VARIANT_METHOD

// void transpose()
START_VARIANT_METHOD( transpose, QSizeF )
   void tmp = value.transpose();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD

// void scale(qreal w, qreal h, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSizeF )
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   Qt::AspectRatioMode mode = static_cast<Qt::AspectRatioMode>(KJSEmbed::extractInt(exec, args, 2, 0));
END_VARIANT_METHOD

// void scale(const QSizeF &s, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSizeF )
   const QSizeF & s = KJSEmbed::extractValue<const QSizeF &>(exec, args, 0);
   Qt::AspectRatioMode mode = static_cast<Qt::AspectRatioMode>(KJSEmbed::extractInt(exec, args, 1, 0));
END_VARIANT_METHOD

// QSizeF expandedTo(const QSizeF &) const 
START_VARIANT_METHOD( expandedTo, QSizeF )
   const QSizeF & arg0 = KJSEmbed::extractValue<const QSizeF &>(exec, args, 0);
END_VARIANT_METHOD

// QSizeF boundedTo(const QSizeF &) const 
START_VARIANT_METHOD( boundedTo, QSizeF )
   const QSizeF & arg0 = KJSEmbed::extractValue<const QSizeF &>(exec, args, 0);
END_VARIANT_METHOD

// qreal & rwidth()
START_VARIANT_METHOD( rwidth, QSizeF )
   qreal & tmp = value.rwidth();
   result = KJSEmbed::createValue( exec, "qreal &", tmp );
END_VARIANT_METHOD

// qreal & rheight()
START_VARIANT_METHOD( rheight, QSizeF )
   qreal & tmp = value.rheight();
   result = KJSEmbed::createValue( exec, "qreal &", tmp );
END_VARIANT_METHOD

// QSize toSize() const 
START_VARIANT_METHOD( toSize, QSizeF )
   QSize tmp = value.toSize();
   result = KJSEmbed::createValue( exec, "QSize", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QSizeF ) 
NO_STATICS( QSizeF ) 



START_CTOR( QSizeF, QSizeF, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QSizeFBinding(exec, QSizeF())
   }
   if (args.size() == 1 )
   {
   const QSize & sz = KJSEmbed::extractValue<const QSize &>(exec, args, 0);
       return new KJSEmbed::QSizeFBinding(exec, QSizeF(sz))
   }
   if (args.size() == 2 )
   {
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
       return new KJSEmbed::QSizeFBinding(exec, QSizeF(w, h))
   }
END_CTOR


START_METHOD_LUT( QSizeF )
    { isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::isNull },
    { isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::isEmpty },
    { isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::isValid },
    { width, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::width },
    { height, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::height },
    { setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::setWidth },
    { setHeight, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::setHeight },
    { transpose, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::transpose },
    { scale, 3, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::scale },
    { scale, 2, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::scale },
    { expandedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::expandedTo },
    { boundedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::boundedTo },
    { rwidth, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::rwidth },
    { rheight, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::rheight },
    { toSize, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::toSize },
END_METHOD_LUT

