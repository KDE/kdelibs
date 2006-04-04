<QtGui>

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

// bool isNull() const 
START_VARIANT_METHOD( isNull, QSize )
   bool tmp = value.isNull();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QSize )
   bool tmp = value.isEmpty();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QSize )
   bool tmp = value.isValid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// int width() const 
START_VARIANT_METHOD( width, QSize )
   int tmp = value.width();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int height() const 
START_VARIANT_METHOD( height, QSize )
   int tmp = value.height();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setWidth(int w)
START_VARIANT_METHOD( setWidth, QSize )
   int w = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setWidth(w);
END_VARIANT_METHOD

// void setHeight(int h)
START_VARIANT_METHOD( setHeight, QSize )
   int h = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setHeight(h);
END_VARIANT_METHOD

// void transpose()
START_VARIANT_METHOD( transpose, QSize )
   void tmp = value.transpose();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD

// void scale(int w, int h, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSize )
   int w = KJSEmbed::extractValue<int>(exec, args, 0);
   int h = KJSEmbed::extractValue<int>(exec, args, 0);
   Qt::AspectRatioMode mode = static_cast<Qt::AspectRatioMode>(KJSEmbed::extractInt(exec, args, 2, 0));
END_VARIANT_METHOD

// void scale(const QSize &s, Qt::AspectRatioMode mode)
START_VARIANT_METHOD( scale, QSize )
   const QSize & s = KJSEmbed::extractValue<const QSize &>(exec, args, 0);
   Qt::AspectRatioMode mode = static_cast<Qt::AspectRatioMode>(KJSEmbed::extractInt(exec, args, 1, 0));
END_VARIANT_METHOD

// QSize expandedTo(const QSize &) const 
START_VARIANT_METHOD( expandedTo, QSize )
   const QSize & arg0 = KJSEmbed::extractValue<const QSize &>(exec, args, 0);
END_VARIANT_METHOD

// QSize boundedTo(const QSize &) const 
START_VARIANT_METHOD( boundedTo, QSize )
   const QSize & arg0 = KJSEmbed::extractValue<const QSize &>(exec, args, 0);
END_VARIANT_METHOD

// int & rwidth()
START_VARIANT_METHOD( rwidth, QSize )
   int & tmp = value.rwidth();
   result = KJSEmbed::createValue( exec, "int &", tmp );
END_VARIANT_METHOD

// int & rheight()
START_VARIANT_METHOD( rheight, QSize )
   int & tmp = value.rheight();
   result = KJSEmbed::createValue( exec, "int &", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QSize ) 
NO_STATICS( QSize ) 



START_CTOR( QSize, QSize, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QSizeBinding(exec, QSize())
   }
   if (args.size() == 2 )
   {
   int w = KJSEmbed::extractValue<int>(exec, args, 0);
   int h = KJSEmbed::extractValue<int>(exec, args, 0);
       return new KJSEmbed::QSizeBinding(exec, QSize(w, h))
   }
END_CTOR


START_METHOD_LUT( QSize )
    { isNull, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::isNull },
    { isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::isEmpty },
    { isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::isValid },
    { width, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::width },
    { height, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::height },
    { setWidth, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::setWidth },
    { setHeight, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::setHeight },
    { transpose, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::transpose },
    { scale, 3, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::scale },
    { scale, 2, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::scale },
    { expandedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::expandedTo },
    { boundedTo, 1, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::boundedTo },
    { rwidth, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::rwidth },
    { rheight, 0, KJS::DontDelete|KJS::ReadOnly, &QSizeNS::rheight },
END_METHOD_LUT

