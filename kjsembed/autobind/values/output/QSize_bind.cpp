#include <QDebug>


#include <QSize_bind.h>

using namespace KJSEmbed;

QSizeBinding::QSize( KJS::ExecState *exec, const QSize &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QSize::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QSizeNS
{

START_CTOR(QSize, QSize, 0 )
   return new KJSEmbed::QSizeBinding(exec, QSize())
END_CTOR

START_CTOR(QSize, QSize, 2 )
   int w = KJSEmbed::extractObject<int>(exec, args, 0, 0);
   int h = KJSEmbed::extractObject<int>(exec, args, 1, 0);
   return new KJSEmbed::QSizeBinding(exec, QSize(w, h))
END_CTOR

// bool isNull() const 
START_VARIANT_METHOD( isNull, QSize )
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QSize )
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QSize )
END_VARIANT_METHOD

// int width() const 
START_VARIANT_METHOD( width, QSize )
END_VARIANT_METHOD

// int height() const 
START_VARIANT_METHOD( height, QSize )
END_VARIANT_METHOD

// void setWidth(int w)
START_VARIANT_METHOD( setWidth, QSize )
   int w = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setHeight(int h)
START_VARIANT_METHOD( setHeight, QSize )
   int h = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// void transpose()
START_VARIANT_METHOD( transpose, QSize )
END_VARIANT_METHOD

// void scale(int w, int h, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSize )
   int w = KJSEmbed::extractObject<int>(exec, args, 0, 0);
   int h = KJSEmbed::extractObject<int>(exec, args, 1, 0);
   Qt::AspectRatioMode mode = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 2, 0);
END_VARIANT_METHOD

// void scale(const QSize &s, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSize )
   const QSize & s = KJSEmbed::extractObject<const QSize &>(exec, args, 0, 0);
   Qt::AspectRatioMode mode = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 1, 0);
END_VARIANT_METHOD

// QSize expandedTo(const QSize &) const 
START_VARIANT_METHOD( expandedTo, QSize )
   const QSize &  = KJSEmbed::extractObject<const QSize &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QSize boundedTo(const QSize &) const 
START_VARIANT_METHOD( boundedTo, QSize )
   const QSize &  = KJSEmbed::extractObject<const QSize &>(exec, args, 0, 0);
END_VARIANT_METHOD

// int & rwidth()
START_VARIANT_METHOD( rwidth, QSize )
END_VARIANT_METHOD

// int & rheight()
START_VARIANT_METHOD( rheight, QSize )
END_VARIANT_METHOD
}


START_METHOD_LUT( QSize)
{QSize, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::QSize },
{QSize, 2, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::QSize },
{isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::isNull },
{isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::isEmpty },
{isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::isValid },
{width, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::width },
{height, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::height },
{setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::setWidth },
{setHeight, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::setHeight },
{transpose, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::transpose },
{scale, 3, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::scale },
{scale, 2, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::scale },
{expandedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::expandedTo },
{boundedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::boundedTo },
{rwidth, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::rwidth },
{rheight, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::rheight },
{operator+=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::operator+= },
{operator-=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::operator-= },
{operator *=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::operator *= },
{operator/=, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::operator/= },
END_METHOD_LUT

