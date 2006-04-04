<QtGui>

#include <QColor_bind.h>

using namespace KJSEmbed;

QColorBinding::QColor( KJS::ExecState *exec, const QColor &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QColor::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QColorNS
{

// bool isValid() const 
START_VARIANT_METHOD( isValid, QColor )
   bool tmp = value.isValid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// QString name() const 
START_VARIANT_METHOD( name, QColor )
   QString tmp = value.name();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setNamedColor(const QString &name)
START_VARIANT_METHOD( setNamedColor, QColor )
   const QString & name = (KJSEmbed::extractObject<const QString &>(exec, args, 0, 0));
END_VARIANT_METHOD

// Spec spec() const 
START_VARIANT_METHOD( spec, QColor )
   Spec tmp = value.spec();
   result = KJSEmbed::createValue( exec, "Spec", tmp );
END_VARIANT_METHOD

// int alpha() const 
START_VARIANT_METHOD( alpha, QColor )
   int tmp = value.alpha();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setAlpha(int alpha)
START_VARIANT_METHOD( setAlpha, QColor )
   int alpha = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
END_VARIANT_METHOD

// qreal alphaF() const 
START_VARIANT_METHOD( alphaF, QColor )
   qreal tmp = value.alphaF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setAlphaF(qreal alpha)
START_VARIANT_METHOD( setAlphaF, QColor )
   qreal alpha = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// int red() const 
START_VARIANT_METHOD( red, QColor )
   int tmp = value.red();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int green() const 
START_VARIANT_METHOD( green, QColor )
   int tmp = value.green();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int blue() const 
START_VARIANT_METHOD( blue, QColor )
   int tmp = value.blue();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setRed(int red)
START_VARIANT_METHOD( setRed, QColor )
   int red = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
END_VARIANT_METHOD

// void setGreen(int green)
START_VARIANT_METHOD( setGreen, QColor )
   int green = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
END_VARIANT_METHOD

// void setBlue(int blue)
START_VARIANT_METHOD( setBlue, QColor )
   int blue = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
END_VARIANT_METHOD

// qreal redF() const 
START_VARIANT_METHOD( redF, QColor )
   qreal tmp = value.redF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal greenF() const 
START_VARIANT_METHOD( greenF, QColor )
   qreal tmp = value.greenF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal blueF() const 
START_VARIANT_METHOD( blueF, QColor )
   qreal tmp = value.blueF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setRedF(qreal red)
START_VARIANT_METHOD( setRedF, QColor )
   qreal red = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// void setGreenF(qreal green)
START_VARIANT_METHOD( setGreenF, QColor )
   qreal green = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// void setBlueF(qreal blue)
START_VARIANT_METHOD( setBlueF, QColor )
   qreal blue = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
END_VARIANT_METHOD

// void getRgb(int *r, int *g, int *b, int *a=0) const 
START_VARIANT_METHOD( getRgb, QColor )
   int * r = (KJSEmbed::extractObject<int *>(exec, args, 0, 0));
   int * g = (KJSEmbed::extractObject<int *>(exec, args, 1, 0));
   int * b = (KJSEmbed::extractObject<int *>(exec, args, 2, 0));
   int * a = (KJSEmbed::extractObject<int *>(exec, args, 3, 0));
END_VARIANT_METHOD

// void setRgb(int r, int g, int b, int a=255)
START_VARIANT_METHOD( setRgb, QColor )
   int r = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
   int g = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
   int b = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
   int a = (KJSEmbed::extractObject<int>(exec, args, 3, 255));
END_VARIANT_METHOD

// void getRgbF(qreal *r, qreal *g, qreal *b, qreal *a=0) const 
START_VARIANT_METHOD( getRgbF, QColor )
   qreal * r = (KJSEmbed::extractObject<qreal *>(exec, args, 0, 0));
   qreal * g = (KJSEmbed::extractObject<qreal *>(exec, args, 1, 0));
   qreal * b = (KJSEmbed::extractObject<qreal *>(exec, args, 2, 0));
   qreal * a = (KJSEmbed::extractObject<qreal *>(exec, args, 3, 0));
END_VARIANT_METHOD

// void setRgbF(qreal r, qreal g, qreal b, qreal a=1.0)
START_VARIANT_METHOD( setRgbF, QColor )
   qreal r = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
   qreal g = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
   qreal b = (KJSEmbed::extractObject<qreal>(exec, args, 2, 0));
   qreal a = (KJSEmbed::extractObject<qreal>(exec, args, 3, 1.0));
END_VARIANT_METHOD

// QRgb rgba() const 
START_VARIANT_METHOD( rgba, QColor )
   QRgb tmp = value.rgba();
   result = KJSEmbed::createValue( exec, "QRgb", tmp );
END_VARIANT_METHOD

// void setRgba(QRgb rgba)
START_VARIANT_METHOD( setRgba, QColor )
   QRgb rgba = (KJSEmbed::extractObject<QRgb>(exec, args, 0, 0));
END_VARIANT_METHOD

// QRgb rgb() const 
START_VARIANT_METHOD( rgb, QColor )
   QRgb tmp = value.rgb();
   result = KJSEmbed::createValue( exec, "QRgb", tmp );
END_VARIANT_METHOD

// void setRgb(QRgb rgb)
START_VARIANT_METHOD( setRgb, QColor )
   QRgb rgb = (KJSEmbed::extractObject<QRgb>(exec, args, 0, 0));
END_VARIANT_METHOD

// int hue() const 
START_VARIANT_METHOD( hue, QColor )
   int tmp = value.hue();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int saturation() const 
START_VARIANT_METHOD( saturation, QColor )
   int tmp = value.saturation();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int value() const 
START_VARIANT_METHOD( value, QColor )
   int tmp = value.value();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// qreal hueF() const 
START_VARIANT_METHOD( hueF, QColor )
   qreal tmp = value.hueF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal saturationF() const 
START_VARIANT_METHOD( saturationF, QColor )
   qreal tmp = value.saturationF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal valueF() const 
START_VARIANT_METHOD( valueF, QColor )
   qreal tmp = value.valueF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void getHsv(int *h, int *s, int *v, int *a=0) const 
START_VARIANT_METHOD( getHsv, QColor )
   int * h = (KJSEmbed::extractObject<int *>(exec, args, 0, 0));
   int * s = (KJSEmbed::extractObject<int *>(exec, args, 1, 0));
   int * v = (KJSEmbed::extractObject<int *>(exec, args, 2, 0));
   int * a = (KJSEmbed::extractObject<int *>(exec, args, 3, 0));
END_VARIANT_METHOD

// void setHsv(int h, int s, int v, int a=255)
START_VARIANT_METHOD( setHsv, QColor )
   int h = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
   int s = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
   int v = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
   int a = (KJSEmbed::extractObject<int>(exec, args, 3, 255));
END_VARIANT_METHOD

// void getHsvF(qreal *h, qreal *s, qreal *v, qreal *a=0) const 
START_VARIANT_METHOD( getHsvF, QColor )
   qreal * h = (KJSEmbed::extractObject<qreal *>(exec, args, 0, 0));
   qreal * s = (KJSEmbed::extractObject<qreal *>(exec, args, 1, 0));
   qreal * v = (KJSEmbed::extractObject<qreal *>(exec, args, 2, 0));
   qreal * a = (KJSEmbed::extractObject<qreal *>(exec, args, 3, 0));
END_VARIANT_METHOD

// void setHsvF(qreal h, qreal s, qreal v, qreal a=1.0)
START_VARIANT_METHOD( setHsvF, QColor )
   qreal h = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
   qreal s = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
   qreal v = (KJSEmbed::extractObject<qreal>(exec, args, 2, 0));
   qreal a = (KJSEmbed::extractObject<qreal>(exec, args, 3, 1.0));
END_VARIANT_METHOD

// int cyan() const 
START_VARIANT_METHOD( cyan, QColor )
   int tmp = value.cyan();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int magenta() const 
START_VARIANT_METHOD( magenta, QColor )
   int tmp = value.magenta();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int yellow() const 
START_VARIANT_METHOD( yellow, QColor )
   int tmp = value.yellow();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int black() const 
START_VARIANT_METHOD( black, QColor )
   int tmp = value.black();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// qreal cyanF() const 
START_VARIANT_METHOD( cyanF, QColor )
   qreal tmp = value.cyanF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal magentaF() const 
START_VARIANT_METHOD( magentaF, QColor )
   qreal tmp = value.magentaF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal yellowF() const 
START_VARIANT_METHOD( yellowF, QColor )
   qreal tmp = value.yellowF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// qreal blackF() const 
START_VARIANT_METHOD( blackF, QColor )
   qreal tmp = value.blackF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void getCmyk(int *c, int *m, int *y, int *k, int *a=0)
START_VARIANT_METHOD( getCmyk, QColor )
   int * c = (KJSEmbed::extractObject<int *>(exec, args, 0, 0));
   int * m = (KJSEmbed::extractObject<int *>(exec, args, 1, 0));
   int * y = (KJSEmbed::extractObject<int *>(exec, args, 2, 0));
   int * k = (KJSEmbed::extractObject<int *>(exec, args, 3, 0));
   int * a = (KJSEmbed::extractObject<int *>(exec, args, 4, 0));
END_VARIANT_METHOD

// void setCmyk(int c, int m, int y, int k, int a=255)
START_VARIANT_METHOD( setCmyk, QColor )
   int c = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
   int m = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
   int y = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
   int k = (KJSEmbed::extractObject<int>(exec, args, 3, 0));
   int a = (KJSEmbed::extractObject<int>(exec, args, 4, 255));
END_VARIANT_METHOD

// void getCmykF(qreal *c, qreal *m, qreal *y, qreal *k, qreal *a=0)
START_VARIANT_METHOD( getCmykF, QColor )
   qreal * c = (KJSEmbed::extractObject<qreal *>(exec, args, 0, 0));
   qreal * m = (KJSEmbed::extractObject<qreal *>(exec, args, 1, 0));
   qreal * y = (KJSEmbed::extractObject<qreal *>(exec, args, 2, 0));
   qreal * k = (KJSEmbed::extractObject<qreal *>(exec, args, 3, 0));
   qreal * a = (KJSEmbed::extractObject<qreal *>(exec, args, 4, 0));
END_VARIANT_METHOD

// void setCmykF(qreal c, qreal m, qreal y, qreal k, qreal a=1.0)
START_VARIANT_METHOD( setCmykF, QColor )
   qreal c = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
   qreal m = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
   qreal y = (KJSEmbed::extractObject<qreal>(exec, args, 2, 0));
   qreal k = (KJSEmbed::extractObject<qreal>(exec, args, 3, 0));
   qreal a = (KJSEmbed::extractObject<qreal>(exec, args, 4, 1.0));
END_VARIANT_METHOD

// QColor toRgb() const 
START_VARIANT_METHOD( toRgb, QColor )
   QColor tmp = value.toRgb();
   result = KJSEmbed::createValue( exec, "QColor", tmp );
END_VARIANT_METHOD

// QColor toHsv() const 
START_VARIANT_METHOD( toHsv, QColor )
   QColor tmp = value.toHsv();
   result = KJSEmbed::createValue( exec, "QColor", tmp );
END_VARIANT_METHOD

// QColor toCmyk() const 
START_VARIANT_METHOD( toCmyk, QColor )
   QColor tmp = value.toCmyk();
   result = KJSEmbed::createValue( exec, "QColor", tmp );
END_VARIANT_METHOD

// QColor convertTo(Spec colorSpec) const 
START_VARIANT_METHOD( convertTo, QColor )
   Spec colorSpec = (KJSEmbed::extractObject<Spec>(exec, args, 0, 0));
END_VARIANT_METHOD

// QColor light(int f=150) const 
START_VARIANT_METHOD( light, QColor )
   int f = (KJSEmbed::extractObject<int>(exec, args, 0, 150));
END_VARIANT_METHOD

// QColor dark(int f=200) const 
START_VARIANT_METHOD( dark, QColor )
   int f = (KJSEmbed::extractObject<int>(exec, args, 0, 200));
END_VARIANT_METHOD

// QStringList colorNames()
START_VARIANT_METHOD( colorNames, QColor )
   QStringList tmp = value.colorNames();
   result = KJSEmbed::createValue( exec, "QStringList", tmp );
END_VARIANT_METHOD

// QColor fromRgb(QRgb rgb)
START_VARIANT_METHOD( fromRgb, QColor )
   QRgb rgb = (KJSEmbed::extractObject<QRgb>(exec, args, 0, 0));
END_VARIANT_METHOD

// QColor fromRgba(QRgb rgba)
START_VARIANT_METHOD( fromRgba, QColor )
   QRgb rgba = (KJSEmbed::extractObject<QRgb>(exec, args, 0, 0));
END_VARIANT_METHOD

// QColor fromRgb(int r, int g, int b, int a=255)
START_VARIANT_METHOD( fromRgb, QColor )
   int r = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
   int g = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
   int b = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
   int a = (KJSEmbed::extractObject<int>(exec, args, 3, 255));
END_VARIANT_METHOD

// QColor fromRgbF(qreal r, qreal g, qreal b, qreal a=1.0)
START_VARIANT_METHOD( fromRgbF, QColor )
   qreal r = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
   qreal g = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
   qreal b = (KJSEmbed::extractObject<qreal>(exec, args, 2, 0));
   qreal a = (KJSEmbed::extractObject<qreal>(exec, args, 3, 1.0));
END_VARIANT_METHOD

// QColor fromHsv(int h, int s, int v, int a=255)
START_VARIANT_METHOD( fromHsv, QColor )
   int h = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
   int s = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
   int v = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
   int a = (KJSEmbed::extractObject<int>(exec, args, 3, 255));
END_VARIANT_METHOD

// QColor fromHsvF(qreal h, qreal s, qreal v, qreal a=1.0)
START_VARIANT_METHOD( fromHsvF, QColor )
   qreal h = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
   qreal s = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
   qreal v = (KJSEmbed::extractObject<qreal>(exec, args, 2, 0));
   qreal a = (KJSEmbed::extractObject<qreal>(exec, args, 3, 1.0));
END_VARIANT_METHOD

// QColor fromCmyk(int c, int m, int y, int k, int a=255)
START_VARIANT_METHOD( fromCmyk, QColor )
   int c = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
   int m = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
   int y = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
   int k = (KJSEmbed::extractObject<int>(exec, args, 3, 0));
   int a = (KJSEmbed::extractObject<int>(exec, args, 4, 255));
END_VARIANT_METHOD

// QColor fromCmykF(qreal c, qreal m, qreal y, qreal k, qreal a=1.0)
START_VARIANT_METHOD( fromCmykF, QColor )
   qreal c = (KJSEmbed::extractObject<qreal>(exec, args, 0, 0));
   qreal m = (KJSEmbed::extractObject<qreal>(exec, args, 1, 0));
   qreal y = (KJSEmbed::extractObject<qreal>(exec, args, 2, 0));
   qreal k = (KJSEmbed::extractObject<qreal>(exec, args, 3, 0));
   qreal a = (KJSEmbed::extractObject<qreal>(exec, args, 4, 1.0));
END_VARIANT_METHOD
}

NO_ENUMS( QColor ) 
NO_STATICS( QColor ) 



START_CTOR( QColor, QColor, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QColorBinding(exec, QColor())
   }
   if (args.size() == 1 )
   {
       Qt::GlobalColor color = static_cast<Qt::GlobalColor>(KJSEmbed::extractInt(exec, args, 0, 0));
       return new KJSEmbed::QColorBinding(exec, QColor(color))
   }
   if (args.size() == 4 )
   {
       int r = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
       int g = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
       int b = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
       int a = (KJSEmbed::extractObject<int>(exec, args, 3, 255));
       return new KJSEmbed::QColorBinding(exec, QColor(r, g, b, a))
   }
   if (args.size() == 1 )
   {
       QRgb rgb = (KJSEmbed::extractObject<QRgb>(exec, args, 0, 0));
       return new KJSEmbed::QColorBinding(exec, QColor(rgb))
   }
   if (args.size() == 1 )
   {
       const QString & name = (KJSEmbed::extractObject<const QString &>(exec, args, 0, 0));
       return new KJSEmbed::QColorBinding(exec, QColor(name))
   }
   if (args.size() == 1 )
   {
       const char * name = (KJSEmbed::extractObject<const char *>(exec, args, 0, 0));
       return new KJSEmbed::QColorBinding(exec, QColor(name))
   }
   if (args.size() == 1 )
   {
       const QColor & color = (KJSEmbed::extractObject<const QColor &>(exec, args, 0, 0));
       return new KJSEmbed::QColorBinding(exec, QColor(color))
   }
   if (args.size() == 4 )
   {
       int  = (KJSEmbed::extractObject<int>(exec, args, 0, 0));
       int  = (KJSEmbed::extractObject<int>(exec, args, 1, 0));
       int  = (KJSEmbed::extractObject<int>(exec, args, 2, 0));
       Spec  = (KJSEmbed::extractObject<Spec>(exec, args, 3, 0));
       return new KJSEmbed::QColorBinding(exec, QColor(, , , ))
   }
END_CTOR


START_METHOD_LUT( QColor )
    { isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::isValid },
    { name, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::name },
    { setNamedColor, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setNamedColor },
    { spec, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::spec },
    { alpha, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::alpha },
    { setAlpha, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setAlpha },
    { alphaF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::alphaF },
    { setAlphaF, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setAlphaF },
    { red, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::red },
    { green, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::green },
    { blue, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::blue },
    { setRed, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setRed },
    { setGreen, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setGreen },
    { setBlue, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setBlue },
    { redF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::redF },
    { greenF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::greenF },
    { blueF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::blueF },
    { setRedF, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setRedF },
    { setGreenF, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setGreenF },
    { setBlueF, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setBlueF },
    { getRgb, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::getRgb },
    { setRgb, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setRgb },
    { getRgbF, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::getRgbF },
    { setRgbF, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setRgbF },
    { rgba, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::rgba },
    { setRgba, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setRgba },
    { rgb, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::rgb },
    { setRgb, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setRgb },
    { hue, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::hue },
    { saturation, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::saturation },
    { value, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::value },
    { hueF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::hueF },
    { saturationF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::saturationF },
    { valueF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::valueF },
    { getHsv, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::getHsv },
    { setHsv, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setHsv },
    { getHsvF, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::getHsvF },
    { setHsvF, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setHsvF },
    { cyan, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::cyan },
    { magenta, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::magenta },
    { yellow, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::yellow },
    { black, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::black },
    { cyanF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::cyanF },
    { magentaF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::magentaF },
    { yellowF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::yellowF },
    { blackF, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::blackF },
    { getCmyk, 5, KJS::DontDelete|KJS::ReadOnly, &QColorNS::getCmyk },
    { setCmyk, 5, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setCmyk },
    { getCmykF, 5, KJS::DontDelete|KJS::ReadOnly, &QColorNS::getCmykF },
    { setCmykF, 5, KJS::DontDelete|KJS::ReadOnly, &QColorNS::setCmykF },
    { toRgb, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::toRgb },
    { toHsv, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::toHsv },
    { toCmyk, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::toCmyk },
    { convertTo, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::convertTo },
    { light, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::light },
    { dark, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::dark },
    { colorNames, 0, KJS::DontDelete|KJS::ReadOnly, &QColorNS::colorNames },
    { fromRgb, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromRgb },
    { fromRgba, 1, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromRgba },
    { fromRgb, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromRgb },
    { fromRgbF, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromRgbF },
    { fromHsv, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromHsv },
    { fromHsvF, 4, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromHsvF },
    { fromCmyk, 5, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromCmyk },
    { fromCmykF, 5, KJS::DontDelete|KJS::ReadOnly, &QColorNS::fromCmykF },
END_METHOD_LUT

