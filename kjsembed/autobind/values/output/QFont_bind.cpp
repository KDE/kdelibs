<QtGui>

#include <QFont_bind.h>

using namespace KJSEmbed;

QFontBinding::QFont( KJS::ExecState *exec, const QFont &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QFont::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QFontNS
{

// QString family() const 
START_VARIANT_METHOD( family, QFont )
   QString tmp = value.family();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setFamily(const QString &)
START_VARIANT_METHOD( setFamily, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setFamily(arg0);
END_VARIANT_METHOD

// int pointSize() const 
START_VARIANT_METHOD( pointSize, QFont )
   int tmp = value.pointSize();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setPointSize(int)
START_VARIANT_METHOD( setPointSize, QFont )
   int arg0 = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setPointSize(arg0);
END_VARIANT_METHOD

// qreal pointSizeF() const 
START_VARIANT_METHOD( pointSizeF, QFont )
   qreal tmp = value.pointSizeF();
   result = KJSEmbed::createValue( exec, "qreal", tmp );
END_VARIANT_METHOD

// void setPointSizeF(qreal)
START_VARIANT_METHOD( setPointSizeF, QFont )
   qreal arg0 = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
    value.setPointSizeF(arg0);
END_VARIANT_METHOD

// int pixelSize() const 
START_VARIANT_METHOD( pixelSize, QFont )
   int tmp = value.pixelSize();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setPixelSize(int)
START_VARIANT_METHOD( setPixelSize, QFont )
   int arg0 = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setPixelSize(arg0);
END_VARIANT_METHOD

// int weight() const 
START_VARIANT_METHOD( weight, QFont )
   int tmp = value.weight();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setWeight(int)
START_VARIANT_METHOD( setWeight, QFont )
   int arg0 = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setWeight(arg0);
END_VARIANT_METHOD

// bool bold() const 
START_VARIANT_METHOD( bold, QFont )
   bool tmp = value.bold();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setBold(bool)
START_VARIANT_METHOD( setBold, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setBold(arg0);
END_VARIANT_METHOD

// void setStyle(Style style)
START_VARIANT_METHOD( setStyle, QFont )
   Style style = KJSEmbed::extractObject<Style>(exec, args, 0, 0);
    value.setStyle(style);
END_VARIANT_METHOD

// Style style() const 
START_VARIANT_METHOD( style, QFont )
   Style tmp = value.style();
   result = KJSEmbed::createValue( exec, "Style", tmp );
END_VARIANT_METHOD

// bool italic() const 
START_VARIANT_METHOD( italic, QFont )
   bool tmp = value.italic();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setItalic(bool b)
START_VARIANT_METHOD( setItalic, QFont )
   bool b = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setItalic(b);
END_VARIANT_METHOD

// bool underline() const 
START_VARIANT_METHOD( underline, QFont )
   bool tmp = value.underline();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setUnderline(bool)
START_VARIANT_METHOD( setUnderline, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setUnderline(arg0);
END_VARIANT_METHOD

// bool overline() const 
START_VARIANT_METHOD( overline, QFont )
   bool tmp = value.overline();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setOverline(bool)
START_VARIANT_METHOD( setOverline, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setOverline(arg0);
END_VARIANT_METHOD

// bool strikeOut() const 
START_VARIANT_METHOD( strikeOut, QFont )
   bool tmp = value.strikeOut();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setStrikeOut(bool)
START_VARIANT_METHOD( setStrikeOut, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setStrikeOut(arg0);
END_VARIANT_METHOD

// bool fixedPitch() const 
START_VARIANT_METHOD( fixedPitch, QFont )
   bool tmp = value.fixedPitch();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setFixedPitch(bool)
START_VARIANT_METHOD( setFixedPitch, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setFixedPitch(arg0);
END_VARIANT_METHOD

// bool kerning() const 
START_VARIANT_METHOD( kerning, QFont )
   bool tmp = value.kerning();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setKerning(bool)
START_VARIANT_METHOD( setKerning, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setKerning(arg0);
END_VARIANT_METHOD

// StyleHint styleHint() const 
START_VARIANT_METHOD( styleHint, QFont )
   StyleHint tmp = value.styleHint();
   result = KJSEmbed::createValue( exec, "StyleHint", tmp );
END_VARIANT_METHOD

// StyleStrategy styleStrategy() const 
START_VARIANT_METHOD( styleStrategy, QFont )
   StyleStrategy tmp = value.styleStrategy();
   result = KJSEmbed::createValue( exec, "StyleStrategy", tmp );
END_VARIANT_METHOD

// void setStyleHint(StyleHint, StyleStrategy=PreferDefault)
START_VARIANT_METHOD( setStyleHint, QFont )
   StyleHint arg0 = KJSEmbed::extractValue<StyleHint>(exec, args, 0);
   StyleStrategy arg1 = KJSEmbed::extractObject<StyleStrategy>(exec, args, 1, PreferDefault);
    value.setStyleHint(arg0);
END_VARIANT_METHOD

// void setStyleStrategy(StyleStrategy s)
START_VARIANT_METHOD( setStyleStrategy, QFont )
   StyleStrategy s = KJSEmbed::extractObject<StyleStrategy>(exec, args, 0, 0);
    value.setStyleStrategy(s);
END_VARIANT_METHOD

// int stretch() const 
START_VARIANT_METHOD( stretch, QFont )
   int tmp = value.stretch();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// void setStretch(int)
START_VARIANT_METHOD( setStretch, QFont )
   int arg0 = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setStretch(arg0);
END_VARIANT_METHOD

// bool rawMode() const 
START_VARIANT_METHOD( rawMode, QFont )
   bool tmp = value.rawMode();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void setRawMode(bool)
START_VARIANT_METHOD( setRawMode, QFont )
   bool arg0 = KJSEmbed::extractValue<bool>(exec, args, 0);
    value.setRawMode(arg0);
END_VARIANT_METHOD

// bool exactMatch() const 
START_VARIANT_METHOD( exactMatch, QFont )
   bool tmp = value.exactMatch();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isCopyOf(const QFont &) const 
START_VARIANT_METHOD( isCopyOf, QFont )
   const QFont & arg0 = KJSEmbed::extractValue<const QFont &>(exec, args, 0);
END_VARIANT_METHOD

// Qt::HANDLE handle() const 
START_VARIANT_METHOD( handle, QFont )
   Qt::HANDLE tmp = value.handle();
   result = KJS::Number( tmp );
END_VARIANT_METHOD

// void setRawName(const QString &)
START_VARIANT_METHOD( setRawName, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setRawName(arg0);
END_VARIANT_METHOD

// QString rawName() const 
START_VARIANT_METHOD( rawName, QFont )
   QString tmp = value.rawName();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// QString key() const 
START_VARIANT_METHOD( key, QFont )
   QString tmp = value.key();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// QString toString() const 
START_VARIANT_METHOD( toString, QFont )
   QString tmp = value.toString();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// bool fromString(const QString &)
START_VARIANT_METHOD( fromString, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QString defaultFamily() const 
START_VARIANT_METHOD( defaultFamily, QFont )
   QString tmp = value.defaultFamily();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// QString lastResortFamily() const 
START_VARIANT_METHOD( lastResortFamily, QFont )
   QString tmp = value.lastResortFamily();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// QString lastResortFont() const 
START_VARIANT_METHOD( lastResortFont, QFont )
   QString tmp = value.lastResortFont();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// QFont resolve(const QFont &) const 
START_VARIANT_METHOD( resolve, QFont )
   const QFont & arg0 = KJSEmbed::extractValue<const QFont &>(exec, args, 0);
END_VARIANT_METHOD

// uint resolve() const 
START_VARIANT_METHOD( resolve, QFont )
   uint tmp = value.resolve();
   result = KJSEmbed::createValue( exec, "uint", tmp );
END_VARIANT_METHOD

// void resolve(uint mask)
START_VARIANT_METHOD( resolve, QFont )
   uint mask = KJSEmbed::extractValue<uint>(exec, args, 0);
END_VARIANT_METHOD

// QString substitute(const QString &)
START_VARIANT_METHOD( substitute, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QStringList substitutes(const QString &)
START_VARIANT_METHOD( substitutes, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QStringList substitutions()
START_VARIANT_METHOD( substitutions, QFont )
   QStringList tmp = value.substitutions();
   result = KJSEmbed::createValue( exec, "QStringList", tmp );
END_VARIANT_METHOD

// void insertSubstitution(const QString &, const QString &)
START_VARIANT_METHOD( insertSubstitution, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   const QString & arg1 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// void insertSubstitutions(const QString &, const QStringList &)
START_VARIANT_METHOD( insertSubstitutions, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   const QStringList & arg1 = KJSEmbed::extractValue<const QStringList &>(exec, args, 0);
END_VARIANT_METHOD

// void removeSubstitution(const QString &)
START_VARIANT_METHOD( removeSubstitution, QFont )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// void initialize()
START_VARIANT_METHOD( initialize, QFont )
   void tmp = value.initialize();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD

// void cleanup()
START_VARIANT_METHOD( cleanup, QFont )
   void tmp = value.cleanup();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD

// void cacheStatistics()
START_VARIANT_METHOD( cacheStatistics, QFont )
   void tmp = value.cacheStatistics();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD
}

NO_ENUMS( QFont ) 
NO_STATICS( QFont ) 



START_CTOR( QFont, QFont, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QFontBinding(exec, QFont())
   }
   if (args.size() == 4 )
   {
   const QString & family = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   int pointSize = KJSEmbed::extractValue<int>(exec, args, -1);
   int weight = KJSEmbed::extractValue<int>(exec, args, -1);
   bool italic = KJSEmbed::extractValue<bool>(exec, args, false);
       return new KJSEmbed::QFontBinding(exec, QFont(family, pointSize, weight, italic))
   }
   if (args.size() == 2 )
   {
   const QFont & arg0 = KJSEmbed::extractValue<const QFont &>(exec, args, 0);
   QPaintDevice * pd = KJSEmbed::extractValue<QPaintDevice *>(exec, args, 0);
       return new KJSEmbed::QFontBinding(exec, QFont(, pd))
   }
   if (args.size() == 1 )
   {
   const QFont & arg0 = KJSEmbed::extractValue<const QFont &>(exec, args, 0);
       return new KJSEmbed::QFontBinding(exec, QFont())
   }
   if (args.size() == 1 )
   {
   QFontPrivate * arg0 = KJSEmbed::extractValue<QFontPrivate *>(exec, args, 0);
       return new KJSEmbed::QFontBinding(exec, QFont())
   }
END_CTOR


START_METHOD_LUT( QFont )
    { family, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::family },
    { setFamily, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setFamily },
    { pointSize, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::pointSize },
    { setPointSize, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setPointSize },
    { pointSizeF, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::pointSizeF },
    { setPointSizeF, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setPointSizeF },
    { pixelSize, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::pixelSize },
    { setPixelSize, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setPixelSize },
    { weight, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::weight },
    { setWeight, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setWeight },
    { bold, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::bold },
    { setBold, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setBold },
    { setStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStyle },
    { style, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::style },
    { italic, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::italic },
    { setItalic, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setItalic },
    { underline, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::underline },
    { setUnderline, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setUnderline },
    { overline, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::overline },
    { setOverline, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setOverline },
    { strikeOut, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::strikeOut },
    { setStrikeOut, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStrikeOut },
    { fixedPitch, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::fixedPitch },
    { setFixedPitch, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setFixedPitch },
    { kerning, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::kerning },
    { setKerning, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setKerning },
    { styleHint, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::styleHint },
    { styleStrategy, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::styleStrategy },
    { setStyleHint, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStyleHint },
    { setStyleStrategy, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStyleStrategy },
    { stretch, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::stretch },
    { setStretch, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStretch },
    { rawMode, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::rawMode },
    { setRawMode, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setRawMode },
    { exactMatch, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::exactMatch },
    { isCopyOf, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::isCopyOf },
    { handle, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::handle },
    { setRawName, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setRawName },
    { rawName, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::rawName },
    { key, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::key },
    { toString, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::toString },
    { fromString, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::fromString },
    { defaultFamily, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::defaultFamily },
    { lastResortFamily, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::lastResortFamily },
    { lastResortFont, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::lastResortFont },
    { resolve, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::resolve },
    { resolve, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::resolve },
    { resolve, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::resolve },
    { substitute, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::substitute },
    { substitutes, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::substitutes },
    { substitutions, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::substitutions },
    { insertSubstitution, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::insertSubstitution },
    { insertSubstitutions, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::insertSubstitutions },
    { removeSubstitution, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::removeSubstitution },
    { initialize, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::initialize },
    { cleanup, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::cleanup },
    { cacheStatistics, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::cacheStatistics },
END_METHOD_LUT

