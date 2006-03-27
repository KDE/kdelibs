#include <QDebug>


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

START_CTOR(QFont, QFont, 0 )
   return new KJSEmbed::QFontBinding(exec, QFont())
END_CTOR

START_CTOR(QFont, QFont, 4 )
   const QString & family = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   int pointSize = KJSEmbed::extractObject<int>(exec, args, 1, -1);
   int weight = KJSEmbed::extractObject<int>(exec, args, 2, -1);
   bool italic = KJSEmbed::extractObject<bool>(exec, args, 3, false);
   return new KJSEmbed::QFontBinding(exec, QFont(family, pointSize, weight, italic))
END_CTOR

START_CTOR(QFont, QFont, 2 )
   const QFont &  = KJSEmbed::extractObject<const QFont &>(exec, args, 0, 0);
   QPaintDevice * pd = KJSEmbed::extractObject<QPaintDevice *>(exec, args, 1, 0);
   return new KJSEmbed::QFontBinding(exec, QFont(, pd))
END_CTOR

START_CTOR(QFont, QFont, 1 )
   const QFont &  = KJSEmbed::extractObject<const QFont &>(exec, args, 0, 0);
   return new KJSEmbed::QFontBinding(exec, QFont())
END_CTOR

START_CTOR(QFont, QFont, 0 )
   return new KJSEmbed::QFontBinding(exec, QFont())
END_CTOR

// QString family() const 
START_VARIANT_METHOD( family, QFont )
END_VARIANT_METHOD

// void setFamily(const QString &)
START_VARIANT_METHOD( setFamily, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// int pointSize() const 
START_VARIANT_METHOD( pointSize, QFont )
END_VARIANT_METHOD

// void setPointSize(int)
START_VARIANT_METHOD( setPointSize, QFont )
   int  = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// qreal pointSizeF() const 
START_VARIANT_METHOD( pointSizeF, QFont )
END_VARIANT_METHOD

// void setPointSizeF(qreal)
START_VARIANT_METHOD( setPointSizeF, QFont )
   qreal  = KJSEmbed::extractObject<qreal>(exec, args, 0, 0);
END_VARIANT_METHOD

// int pixelSize() const 
START_VARIANT_METHOD( pixelSize, QFont )
END_VARIANT_METHOD

// void setPixelSize(int)
START_VARIANT_METHOD( setPixelSize, QFont )
   int  = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// int weight() const 
START_VARIANT_METHOD( weight, QFont )
END_VARIANT_METHOD

// void setWeight(int)
START_VARIANT_METHOD( setWeight, QFont )
   int  = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool bold() const 
START_VARIANT_METHOD( bold, QFont )
END_VARIANT_METHOD

// void setBold(bool)
START_VARIANT_METHOD( setBold, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setStyle(Style style)
START_VARIANT_METHOD( setStyle, QFont )
   Style style = KJSEmbed::extractObject<Style>(exec, args, 0, 0);
END_VARIANT_METHOD

// Style style() const 
START_VARIANT_METHOD( style, QFont )
END_VARIANT_METHOD

// bool italic() const 
START_VARIANT_METHOD( italic, QFont )
END_VARIANT_METHOD

// void setItalic(bool b)
START_VARIANT_METHOD( setItalic, QFont )
   bool b = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool underline() const 
START_VARIANT_METHOD( underline, QFont )
END_VARIANT_METHOD

// void setUnderline(bool)
START_VARIANT_METHOD( setUnderline, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool overline() const 
START_VARIANT_METHOD( overline, QFont )
END_VARIANT_METHOD

// void setOverline(bool)
START_VARIANT_METHOD( setOverline, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool strikeOut() const 
START_VARIANT_METHOD( strikeOut, QFont )
END_VARIANT_METHOD

// void setStrikeOut(bool)
START_VARIANT_METHOD( setStrikeOut, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool fixedPitch() const 
START_VARIANT_METHOD( fixedPitch, QFont )
END_VARIANT_METHOD

// void setFixedPitch(bool)
START_VARIANT_METHOD( setFixedPitch, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool kerning() const 
START_VARIANT_METHOD( kerning, QFont )
END_VARIANT_METHOD

// void setKerning(bool)
START_VARIANT_METHOD( setKerning, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// StyleHint styleHint() const 
START_VARIANT_METHOD( styleHint, QFont )
END_VARIANT_METHOD

// StyleStrategy styleStrategy() const 
START_VARIANT_METHOD( styleStrategy, QFont )
END_VARIANT_METHOD

// void setStyleHint(StyleHint, StyleStrategy=PreferDefault)
START_VARIANT_METHOD( setStyleHint, QFont )
   StyleHint  = KJSEmbed::extractObject<StyleHint>(exec, args, 0, 0);
   StyleStrategy  = KJSEmbed::extractObject<StyleStrategy>(exec, args, 1, PreferDefault);
END_VARIANT_METHOD

// void setStyleStrategy(StyleStrategy s)
START_VARIANT_METHOD( setStyleStrategy, QFont )
   StyleStrategy s = KJSEmbed::extractObject<StyleStrategy>(exec, args, 0, 0);
END_VARIANT_METHOD

// int stretch() const 
START_VARIANT_METHOD( stretch, QFont )
END_VARIANT_METHOD

// void setStretch(int)
START_VARIANT_METHOD( setStretch, QFont )
   int  = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool rawMode() const 
START_VARIANT_METHOD( rawMode, QFont )
END_VARIANT_METHOD

// void setRawMode(bool)
START_VARIANT_METHOD( setRawMode, QFont )
   bool  = KJSEmbed::extractObject<bool>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool exactMatch() const 
START_VARIANT_METHOD( exactMatch, QFont )
END_VARIANT_METHOD

// bool isCopyOf(const QFont &) const 
START_VARIANT_METHOD( isCopyOf, QFont )
   const QFont &  = KJSEmbed::extractObject<const QFont &>(exec, args, 0, 0);
END_VARIANT_METHOD

// Qt::HANDLE handle() const 
START_VARIANT_METHOD( handle, QFont )
END_VARIANT_METHOD

// void setRawName(const QString &)
START_VARIANT_METHOD( setRawName, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString rawName() const 
START_VARIANT_METHOD( rawName, QFont )
END_VARIANT_METHOD

// QString key() const 
START_VARIANT_METHOD( key, QFont )
END_VARIANT_METHOD

// QString toString() const 
START_VARIANT_METHOD( toString, QFont )
END_VARIANT_METHOD

// bool fromString(const QString &)
START_VARIANT_METHOD( fromString, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString defaultFamily() const 
START_VARIANT_METHOD( defaultFamily, QFont )
END_VARIANT_METHOD

// QString lastResortFamily() const 
START_VARIANT_METHOD( lastResortFamily, QFont )
END_VARIANT_METHOD

// QString lastResortFont() const 
START_VARIANT_METHOD( lastResortFont, QFont )
END_VARIANT_METHOD

// QFont resolve(const QFont &) const 
START_VARIANT_METHOD( resolve, QFont )
   const QFont &  = KJSEmbed::extractObject<const QFont &>(exec, args, 0, 0);
END_VARIANT_METHOD

// uint resolve() const 
START_VARIANT_METHOD( resolve, QFont )
END_VARIANT_METHOD

// void resolve(uint mask)
START_VARIANT_METHOD( resolve, QFont )
   uint mask = KJSEmbed::extractObject<uint>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString substitute(const QString &)
START_VARIANT_METHOD( substitute, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QStringList substitutes(const QString &)
START_VARIANT_METHOD( substitutes, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QStringList substitutions()
START_VARIANT_METHOD( substitutions, QFont )
END_VARIANT_METHOD

// void insertSubstitution(const QString &, const QString &)
START_VARIANT_METHOD( insertSubstitution, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 1, 0);
END_VARIANT_METHOD

// void insertSubstitutions(const QString &, const QStringList &)
START_VARIANT_METHOD( insertSubstitutions, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   const QStringList &  = KJSEmbed::extractObject<const QStringList &>(exec, args, 1, 0);
END_VARIANT_METHOD

// void removeSubstitution(const QString &)
START_VARIANT_METHOD( removeSubstitution, QFont )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void initialize()
START_VARIANT_METHOD( initialize, QFont )
END_VARIANT_METHOD

// void cleanup()
START_VARIANT_METHOD( cleanup, QFont )
END_VARIANT_METHOD

// void cacheStatistics()
START_VARIANT_METHOD( cacheStatistics, QFont )
END_VARIANT_METHOD

START_CTOR(QFont, QFont, 1 )
   QFontPrivate *  = KJSEmbed::extractObject<QFontPrivate *>(exec, args, 0, 0);
   return new KJSEmbed::QFontBinding(exec, QFont())
END_CTOR

// void detach()
START_VARIANT_METHOD( detach, QFont )
END_VARIANT_METHOD
}


START_METHOD_LUT( QFont)
{QFont, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::QFont },
{QFont, 4, KJS::DontDelete|KJS::ReadOnly, &QFontNS::QFont },
{QFont, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::QFont },
{QFont, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::QFont },
{~QFont, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::~QFont },
{family, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::family },
{setFamily, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setFamily },
{pointSize, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::pointSize },
{setPointSize, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setPointSize },
{pointSizeF, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::pointSizeF },
{setPointSizeF, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setPointSizeF },
{pixelSize, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::pixelSize },
{setPixelSize, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setPixelSize },
{weight, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::weight },
{setWeight, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setWeight },
{bold, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::bold },
{setBold, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setBold },
{setStyle, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStyle },
{style, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::style },
{italic, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::italic },
{setItalic, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setItalic },
{underline, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::underline },
{setUnderline, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setUnderline },
{overline, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::overline },
{setOverline, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setOverline },
{strikeOut, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::strikeOut },
{setStrikeOut, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStrikeOut },
{fixedPitch, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::fixedPitch },
{setFixedPitch, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setFixedPitch },
{kerning, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::kerning },
{setKerning, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setKerning },
{styleHint, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::styleHint },
{styleStrategy, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::styleStrategy },
{setStyleHint, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStyleHint },
{setStyleStrategy, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStyleStrategy },
{stretch, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::stretch },
{setStretch, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setStretch },
{rawMode, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::rawMode },
{setRawMode, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setRawMode },
{exactMatch, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::exactMatch },
{operator=, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::operator= },
{operator==, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::operator== },
{operator!=, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::operator!= },
{operator<, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::operator< },
{operator QVariant, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::operator QVariant },
{isCopyOf, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::isCopyOf },
{handle, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::handle },
{setRawName, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::setRawName },
{rawName, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::rawName },
{key, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::key },
{toString, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::toString },
{fromString, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::fromString },
{defaultFamily, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::defaultFamily },
{lastResortFamily, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::lastResortFamily },
{lastResortFont, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::lastResortFont },
{resolve, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::resolve },
{resolve, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::resolve },
{resolve, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::resolve },
{substitute, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::substitute },
{substitutes, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::substitutes },
{substitutions, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::substitutions },
{insertSubstitution, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::insertSubstitution },
{insertSubstitutions, 2, KJS::DontDelete|KJS::ReadOnly, &QFontNS::insertSubstitutions },
{removeSubstitution, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::removeSubstitution },
{initialize, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::initialize },
{cleanup, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::cleanup },
{cacheStatistics, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::cacheStatistics },
{QFont, 1, KJS::DontDelete|KJS::ReadOnly, &QFontNS::QFont },
{detach, 0, KJS::DontDelete|KJS::ReadOnly, &QFontNS::detach },
END_METHOD_LUT

