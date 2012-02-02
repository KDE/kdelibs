/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "color.h"

#include <QtCore/QDebug>
#include <QColor>

using namespace KJSEmbed;

const KJS::ClassInfo ColorBinding::info = { "QColor", &VariantBinding::info, 0, 0 };
ColorBinding::ColorBinding( KJS::ExecState *exec, const QColor &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Color::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

START_VARIANT_METHOD( callSetAlpha, QColor )
    value.setAlpha( KJSEmbed::extractInt(exec, args, 0) );
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetBlue, QColor )
    value.setBlue( KJSEmbed::extractInt(exec, args, 0) );
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetGreen, QColor )
    value.setGreen( KJSEmbed::extractInt(exec, args, 0) );
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetRed, QColor )
    value.setRed( KJSEmbed::extractInt(exec, args, 0) );
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetRgb, QColor )
    value.setRgb( KJSEmbed::extractInt(exec, args, 0),
                  KJSEmbed::extractInt(exec, args, 1),
                  KJSEmbed::extractInt(exec, args, 2),
                  KJSEmbed::extractInt(exec, args, 3, 255));
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetCmyk, QColor )
    value.setCmyk( KJSEmbed::extractInt(exec, args, 0),
                   KJSEmbed::extractInt(exec, args, 1),
                   KJSEmbed::extractInt(exec, args, 2),
                   KJSEmbed::extractInt(exec, args, 3),
                   KJSEmbed::extractInt(exec, args, 4,255));
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetHsv, QColor )
    value.setHsv( KJSEmbed::extractInt(exec, args, 0),
                  KJSEmbed::extractInt(exec, args, 1),
                  KJSEmbed::extractInt(exec, args, 2),
                  KJSEmbed::extractInt(exec, args, 3,255));
END_VARIANT_METHOD

START_VARIANT_METHOD( callSetNamedColor, QColor )
    value.setNamedColor( KJSEmbed::extractQString(exec, args, 0) );
END_VARIANT_METHOD


START_VARIANT_METHOD( callAlpha, QColor )
    value.setAlpha( KJSEmbed::extractInt(exec, args, 0) );
END_VARIANT_METHOD

START_VARIANT_METHOD( callBlue, QColor )
    result = KJSEmbed::createInt( exec, value.blue() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callCyan, QColor )
    result = KJSEmbed::createInt( exec, value.cyan() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callGreen, QColor )
    result = KJSEmbed::createInt( exec, value.green() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callHue, QColor )
    result = KJSEmbed::createInt( exec, value.hue() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callMagenta, QColor )
    result = KJSEmbed::createInt( exec, value.magenta() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callRed, QColor )
    result = KJSEmbed::createInt( exec, value.red() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callYellow, QColor )
    result = KJSEmbed::createInt( exec, value.yellow() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callSaturation, QColor )
    result = KJSEmbed::createInt( exec, value.saturation() );
END_VARIANT_METHOD

START_VARIANT_METHOD( callDark, QColor )
    QColor darkColor = value.dark( KJSEmbed::extractInt( exec, args, 0, 200));
    result = KJSEmbed::createVariant(exec, "QColor", darkColor);
END_VARIANT_METHOD

START_VARIANT_METHOD( callLight, QColor )
    QColor darkColor = value.light( KJSEmbed::extractInt( exec, args, 0, 200));
    result = KJSEmbed::createVariant(exec, "QColor", darkColor);
END_VARIANT_METHOD

START_VARIANT_METHOD( callConvertTo, QColor )
    QColor otherColor = value.convertTo( (QColor::Spec)KJSEmbed::extractInt( exec, args, 0));
    result = KJSEmbed::createVariant(exec, "QColor", otherColor);
END_VARIANT_METHOD

START_VARIANT_METHOD( callSpec, QColor )
    result = KJS::jsNumber( value.spec() );
END_VARIANT_METHOD

START_METHOD_LUT( Color )
    {"setAlpha", 1, KJS::DontDelete|KJS::ReadOnly, &callSetAlpha},
    {"setBlue", 1, KJS::DontDelete|KJS::ReadOnly, &callSetBlue},
    {"setGreen", 1, KJS::DontDelete|KJS::ReadOnly, &callSetGreen},
    {"setRed", 1, KJS::DontDelete|KJS::ReadOnly, &callSetRed},
    {"setRgb", 4, KJS::DontDelete|KJS::ReadOnly, &callSetRgb},
    {"setCmyk", 5, KJS::DontDelete|KJS::ReadOnly, &callSetCmyk},
    {"setHsv", 4, KJS::DontDelete|KJS::ReadOnly, &callSetHsv},
    {"setNamedColor", 1, KJS::DontDelete|KJS::ReadOnly, &callSetNamedColor},
    {"alpha", 0, KJS::DontDelete|KJS::ReadOnly, &callAlpha},
    {"blue", 0, KJS::DontDelete|KJS::ReadOnly, &callBlue},
    {"cyan", 0, KJS::DontDelete|KJS::ReadOnly, &callCyan},
    {"green", 0, KJS::DontDelete|KJS::ReadOnly, &callGreen},
    {"hue", 0, KJS::DontDelete|KJS::ReadOnly, &callHue},
    {"magenta", 0, KJS::DontDelete|KJS::ReadOnly, &callMagenta},
    {"red", 0, KJS::DontDelete|KJS::ReadOnly, &callRed},
    {"saturation", 0, KJS::DontDelete|KJS::ReadOnly, &callSaturation},
    {"yellow", 0, KJS::DontDelete|KJS::ReadOnly, &callYellow},
    {"light", 1, KJS::DontDelete|KJS::ReadOnly, &callLight},
    {"dark", 1, KJS::DontDelete|KJS::ReadOnly, &callDark},
    {"convertTo", 1, KJS::DontDelete|KJS::ReadOnly, &callConvertTo},
    {"spec", 0, KJS::DontDelete|KJS::ReadOnly, &callSpec}
END_METHOD_LUT


START_ENUM_LUT( Color )
    {"Rgb",QColor::Rgb},
    {"Hsv",QColor::Hsv},
    {"Cmyk",QColor::Cmyk},
    {"Invalid",QColor::Invalid}
END_ENUM_LUT

NO_STATICS( Color )

START_CTOR( Color, QColor, 0)
    if( args.size() == 1 )
    {
        return new KJSEmbed::ColorBinding( exec, QColor( KJSEmbed::extractQString(exec,args,0 ) ) );
    }
    else if( args.size() >= 3 )
    {
        return new KJSEmbed::ColorBinding(exec,
                    QColor( KJSEmbed::extractInt( exec, args, 0 ),
                            KJSEmbed::extractInt( exec, args, 1 ),
                            KJSEmbed::extractInt( exec, args, 2 )) );
    }

    if( args.size() == 4 )
    {
        return new KJSEmbed::ColorBinding(exec,
                    QColor( KJSEmbed::extractInt( exec, args, 0 ),
                            KJSEmbed::extractInt( exec, args, 1 ),
                            KJSEmbed::extractInt( exec, args, 2 ),
                            KJSEmbed::extractInt( exec, args, 3 )) );
    }

    return new KJSEmbed::ColorBinding( exec, QColor() );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
