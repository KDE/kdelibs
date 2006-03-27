#include <QDebug>


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

START_CTOR(QSizeF, QSizeF, 0 )
   return new KJSEmbed::QSizeFBinding(exec, QSizeF())
END_CTOR

START_CTOR(QSizeF, QSizeF, 1 )
   const QSize & sz = KJSEmbed::extractObject<const QSize &>(exec, args, 0, 0);
   return new KJSEmbed::QSizeFBinding(exec, QSizeF(sz))
END_CTOR

START_CTOR(QSizeF, QSizeF, 2 )
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   return new KJSEmbed::QSizeFBinding(exec, QSizeF(w, h))
END_CTOR

// bool isNull() const 
START_VARIANT_METHOD( isNull, QSizeF )
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QSizeF )
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QSizeF )
END_VARIANT_METHOD

// qreal width() const 
START_VARIANT_METHOD( width, QSizeF )
END_VARIANT_METHOD

// qreal height() const 
START_VARIANT_METHOD( height, QSizeF )
END_VARIANT_METHOD

// void setWidth(qreal w)
START_VARIANT_METHOD( setWidth, QSizeF )
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setHeight(qreal h)
START_VARIANT_METHOD( setHeight, QSizeF )
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// void transpose()
START_VARIANT_METHOD( transpose, QSizeF )
END_VARIANT_METHOD

// void scale(qreal w, qreal h, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSizeF )
   qreal w = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
   qreal h = KJSEmbed::extractObject<qreal>(exec, args, 1, 0);
   Qt::AspectRatioMode mode = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 2, 0);
END_VARIANT_METHOD

// void scale(const QSizeF &s, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSizeF )
   const QSizeF & s = KJSEmbed::extractObject<const QSizeF &>(exec, args, 0, 0);
   Qt::AspectRatioMode mode = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 1, 0);
END_VARIANT_METHOD

// QSizeF expandedTo(const QSizeF &) const 
START_VARIANT_METHOD( expandedTo, QSizeF )
   const QSizeF &  = KJSEmbed::extractObject<const QSizeF &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QSizeF boundedTo(const QSizeF &) const 
START_VARIANT_METHOD( boundedTo, QSizeF )
   const QSizeF &  = KJSEmbed::extractObject<const QSizeF &>(exec, args, 0, 0);
END_VARIANT_METHOD

// qreal & rwidth()
START_VARIANT_METHOD( rwidth, QSizeF )
END_VARIANT_METHOD

// qreal & rheight()
START_VARIANT_METHOD( rheight, QSizeF )
END_VARIANT_METHOD

// QSize toSize() const 
START_VARIANT_METHOD( toSize, QSizeF )
END_VARIANT_METHOD
}


START_METHOD_LUT( QSizeF)
{QSizeF, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::QSizeF },
{QSizeF, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::QSizeF },
{QSizeF, 2, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::QSizeF },
{isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::isNull },
{isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::isEmpty },
{isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::isValid },
{width, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::width },
{height, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::height },
{setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::setWidth },
{setHeight, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::setHeight },
{transpose, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::transpose },
{scale, 3, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::scale },
{scale, 2, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::scale },
{expandedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::expandedTo },
{boundedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::boundedTo },
{rwidth, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::rwidth },
{rheight, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::rheight },
{operator+=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::operator+= },
{operator-=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::operator-= },
{operator *=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::operator *= },
{operator/=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::operator/= },
{toSize, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeFNS::toSize },
END_METHOD_LUT

