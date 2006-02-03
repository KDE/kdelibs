#include "font.h"

#include <QDebug>
#include <QFont>
#include <QStringList>
using namespace KJSEmbed;

FontBinding::FontBinding( KJS::ExecState *exec, const QFont &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, Font::methods() );
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

namespace FontNS
{

START_VARIANT_METHOD( callbold, QFont )
    bool cppValue = value.bold();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callexactMatch, QFont )
    bool cppValue = value.exactMatch();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callfamily, QFont )
    QString cppValue = value.family();
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callfixedPitch, QFont )
    bool cppValue = value.fixedPitch();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callfromString, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    bool cppValue = value.fromString(arg0);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callinsertSubstitution, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QString arg1 = KJSEmbed::extractQString(exec, args, 1);
    value.insertSubstitution(arg0, arg1);
END_VARIANT_METHOD

START_VARIANT_METHOD( callinsertSubstitutions, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QStringList arg1 = KJSEmbed::extractQStringList(exec,args, 1);
    value.insertSubstitutions(arg0, arg1);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisCopyOf, QFont )
    QFont arg0 = KJSEmbed::extractValue<QFont>(exec,args, 0);
    bool cppValue = value.isCopyOf(arg0);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callitalic, QFont )
    bool cppValue = value.italic();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callkerning, QFont )
    bool cppValue = value.kerning();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callkey, QFont )
    QString cppValue = value.key();
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calllastResortFamily, QFont )
    QString cppValue = value.lastResortFamily();
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calllastResortFont, QFont )
    QString cppValue = value.lastResortFont();
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calloverline, QFont )
    bool cppValue = value.overline();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callpixelSize, QFont )
    int cppValue = value.pixelSize();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callpointSize, QFont )
    int cppValue = value.pointSize();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrawMode, QFont )
    bool cppValue = value.rawMode();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrawName, QFont )
    QString cppValue = value.rawName();
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callremoveSubstitution, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.removeSubstitution(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callresolve, QFont )
    QFont arg0 = KJSEmbed::extractValue<QFont>(exec,args, 0);
    QFont cppValue = value.resolve(arg0);
    result = KJSEmbed::createValue(exec, "QFont", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetBold, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setBold(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetFamily, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.setFamily(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetFixedPitch, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setFixedPitch(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetItalic, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setItalic(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetKerning, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setKerning(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetOverline, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setOverline(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetPixelSize, QFont )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setPixelSize(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetPointSize, QFont )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setPointSize(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetRawMode, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setRawMode(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetRawName, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    value.setRawName(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStretch, QFont )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setStretch(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStrikeOut, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setStrikeOut(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStyle, QFont )
    QFont::Style arg0 = (QFont::Style) KJSEmbed::extractInt(exec, args, 0);
    value.setStyle(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStyleHint, QFont )
    QFont::StyleHint arg0  = (QFont::StyleHint) KJSEmbed::extractInt(exec, args, 0);
    QFont::StyleStrategy arg1  = (QFont::StyleStrategy) KJSEmbed::extractInt(exec, args, 1);
    value.setStyleHint(arg0, arg1);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStyleStrategy, QFont )
    QFont::StyleStrategy arg0  = (QFont::StyleStrategy) KJSEmbed::extractInt(exec, args, 0);
    value.setStyleStrategy(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetUnderline, QFont )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    value.setUnderline(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetWeight, QFont )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setWeight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstretch, QFont )
    int cppValue = value.stretch();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstrikeOut, QFont )
    bool cppValue = value.strikeOut();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstyle, QFont )
    QFont::Style cppValue = value.style();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstyleHint, QFont )
    QFont::StyleHint cppValue = value.styleHint();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstyleStrategy, QFont )
    QFont::StyleStrategy cppValue = value.styleStrategy();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsubstitute, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QString cppValue = value.substitute(arg0);
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsubstitutes, QFont )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QStringList cppValue = value.substitutes(arg0);
    result = KJSEmbed::createQStringList(exec, cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsubstitutions, QFont )
    QStringList cppValue = value.substitutions();
    result = KJSEmbed::createQStringList(exec, cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calltoString, QFont )
    QString cppValue = value.toString();
    result = KJS::String( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callunderline, QFont )
    bool cppValue = value.underline();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callweight, QFont )
    int cppValue = value.weight();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD
}

START_METHOD_LUT( Font )
    {"bold", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callbold},
    {"exactMatch", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callexactMatch},
    {"family", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callfamily},
    {"fixedPitch", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callfixedPitch},
    {"fromString", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callfromString},
    {"insertSubstitution", 1, KJS::DontDelete|KJS::ReadOnly, &FontNS::callinsertSubstitution},
    {"insertSubstitutions", 1, KJS::DontDelete|KJS::ReadOnly, &FontNS::callinsertSubstitutions},
    {"isCopyOf", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callisCopyOf},
    {"italic", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callitalic},
    {"kerning", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callkerning},
    {"key", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callkey},
    {"lastResortFamily", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::calllastResortFamily},
    {"lastResortFont", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::calllastResortFont},
    {"overline", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::calloverline},
    {"pixelSize", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callpixelSize},
    {"pointSize", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callpointSize},
    {"rawMode", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callrawMode},
    {"rawName", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callrawName},
    {"removeSubstitution", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callremoveSubstitution},
    {"resolve", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callresolve},
    {"setBold", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetBold},
    {"setFamily", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetFamily},
    {"setFixedPitch", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetFixedPitch},
    {"setItalic", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetItalic},
    {"setKerning", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetKerning},
    {"setOverline", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetOverline},
    {"setPixelSize", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetPixelSize},
    {"setPointSize", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetPointSize},
    {"setRawMode", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetRawMode},
    {"setRawName", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetRawName},
    {"setStretch", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetStretch},
    {"setStrikeOut", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetStrikeOut},
    {"setStyle", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetStyle},
    {"setStyleHint", 1, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetStyleHint},
    {"setStyleStrategy", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetStyleStrategy},
    {"setUnderline", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetUnderline},
    {"setWeight", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsetWeight},
    {"stretch", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callstretch},
    {"strikeOut", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callstrikeOut},
    {"style", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callstyle},
    {"styleHint", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callstyleHint},
    {"styleStrategy", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callstyleStrategy},
    {"substitute", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsubstitute},
    {"substitutes", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsubstitutes},
    {"substitutions", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callsubstitutions},
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::calltoString},
    {"underline", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callunderline},
    {"weight", 0, KJS::DontDelete|KJS::ReadOnly, &FontNS::callweight}
END_METHOD_LUT

NO_ENUMS( Font )
NO_STATICS( Font )

START_CTOR( Font, QFont, 0)
    if( args.size() == 0 )
    {
        return new KJSEmbed::FontBinding( exec, QFont() );
    }
    return new KJSEmbed::FontBinding( exec, QFont() );
END_CTOR
