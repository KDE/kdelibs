#include <QDebug>


#include <QBrush_bind.h>

using namespace KJSEmbed;

QBrushBinding::QBrush( KJS::ExecState *exec, const QBrush &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QBrush::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QBrushNS
{

START_CTOR(QBrush, QBrush, 0 )
   return new KJSEmbed::QBrushBinding(exec, QBrush())
END_CTOR

START_CTOR(QBrush, QBrush, 1 )
   Qt::BrushStyle bs = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 0, 0);
   return new KJSEmbed::QBrushBinding(exec, QBrush(bs))
END_CTOR

START_CTOR(QBrush, QBrush, 2 )
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0);
   Qt::BrushStyle bs = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 1, Qt::SolidPattern);
   return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs))
END_CTOR

START_CTOR(QBrush, QBrush, 2 )
   Qt::GlobalColor color = (Qt::GlobalColor)KJSEmbed::extractInt(exec, args, 0, 0);
   Qt::BrushStyle bs = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 1, Qt::SolidPattern);
   return new KJSEmbed::QBrushBinding(exec, QBrush(color, bs))
END_CTOR

START_CTOR(QBrush, QBrush, 2 )
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0);
   const QPixmap & pixmap = KJSEmbed::extractObject<const QPixmap &>(exec, args, 1, 0);
   return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap))
END_CTOR

START_CTOR(QBrush, QBrush, 2 )
   Qt::GlobalColor color = (Qt::GlobalColor)KJSEmbed::extractInt(exec, args, 0, 0);
   const QPixmap & pixmap = KJSEmbed::extractObject<const QPixmap &>(exec, args, 1, 0);
   return new KJSEmbed::QBrushBinding(exec, QBrush(color, pixmap))
END_CTOR

START_CTOR(QBrush, QBrush, 1 )
   const QPixmap & pixmap = KJSEmbed::extractObject<const QPixmap &>(exec, args, 0, 0);
   return new KJSEmbed::QBrushBinding(exec, QBrush(pixmap))
END_CTOR

START_CTOR(QBrush, QBrush, 1 )
   const QBrush & brush = KJSEmbed::extractObject<const QBrush &>(exec, args, 0, 0);
   return new KJSEmbed::QBrushBinding(exec, QBrush(brush))
END_CTOR

START_CTOR(QBrush, QBrush, 1 )
   const QGradient & gradient = KJSEmbed::extractObject<const QGradient &>(exec, args, 0, 0);
   return new KJSEmbed::QBrushBinding(exec, QBrush(gradient))
END_CTOR

START_CTOR(QBrush, QBrush, 0 )
   return new KJSEmbed::QBrushBinding(exec, QBrush())
END_CTOR

// Qt::BrushStyle style() const 
START_VARIANT_METHOD( style, QBrush )
END_VARIANT_METHOD

// void setStyle(Qt::BrushStyle)
START_VARIANT_METHOD( setStyle, QBrush )
   Qt::BrushStyle  = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 0, 0);
END_VARIANT_METHOD

// QPixmap texture() const 
START_VARIANT_METHOD( texture, QBrush )
END_VARIANT_METHOD

// void setTexture(const QPixmap &pixmap)
START_VARIANT_METHOD( setTexture, QBrush )
   const QPixmap & pixmap = KJSEmbed::extractObject<const QPixmap &>(exec, args, 0, 0);
END_VARIANT_METHOD

// const QColor & color() const 
START_VARIANT_METHOD( color, QBrush )
END_VARIANT_METHOD

// void setColor(const QColor &color)
START_VARIANT_METHOD( setColor, QBrush )
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setColor(Qt::GlobalColor color)
START_VARIANT_METHOD( setColor, QBrush )
   Qt::GlobalColor color = (Qt::GlobalColor)KJSEmbed::extractInt(exec, args, 0, 0);
END_VARIANT_METHOD

// const QGradient * gradient() const 
START_VARIANT_METHOD( gradient, QBrush )
END_VARIANT_METHOD

// bool isOpaque() const 
START_VARIANT_METHOD( isOpaque, QBrush )
END_VARIANT_METHOD

// void detach(Qt::BrushStyle newStyle)
START_VARIANT_METHOD( detach, QBrush )
   Qt::BrushStyle newStyle = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 0, 0);
END_VARIANT_METHOD

// void init(const QColor &color, Qt::BrushStyle bs)
START_VARIANT_METHOD( init, QBrush )
   const QColor & color = KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0);
   Qt::BrushStyle bs = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 1, 0);
END_VARIANT_METHOD

// void cleanUp(QBrushData *x)
START_VARIANT_METHOD( cleanUp, QBrush )
   QBrushData * x = KJSEmbed::extractObject<QBrushData *>(exec, args, 0, 0);
END_VARIANT_METHOD
}


START_METHOD_LUT( QBrush)
{QBrush, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 2, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 2, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 2, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 2, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{QBrush, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::QBrush },
{~QBrush, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::~QBrush },
{operator=, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::operator= },
{operator QVariant, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::operator QVariant },
{style, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::style },
{setStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setStyle },
{texture, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::texture },
{setTexture, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setTexture },
{color, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::color },
{setColor, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
{setColor, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::setColor },
{gradient, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::gradient },
{isOpaque, 0, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::isOpaque },
{operator==, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::operator== },
{operator!=, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::operator!= },
{detach, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::detach },
{init, 2, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::init },
{cleanUp, 1, KJS::DontDelete|KJS::ReadOnly, &QBrushNS::cleanUp },
END_METHOD_LUT

